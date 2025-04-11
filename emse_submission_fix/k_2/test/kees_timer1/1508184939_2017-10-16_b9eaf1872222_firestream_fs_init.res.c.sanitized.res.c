static int fs_init(struct fs_dev *dev)
{
  struct pci_dev *pci_dev;
  int isr, to;
  int i;
  func_enter();
  pci_dev = dev->pci_dev;
  printk(KERN_INFO "found a FireStream %d card, base %16llx, irq%d.\n", IS_FS50(dev) ? 50 : 155, (unsigned long long)pci_resource_start(pci_dev, 0), dev->pci_dev->irq);
  if (fs_debug & FS_DEBUG_INIT)
    my_hd((unsigned char *)dev, sizeof(*dev));
  undocumented_pci_fix(pci_dev);
  dev->hw_base = pci_resource_start(pci_dev, 0);
  dev->base = ioremap(dev->hw_base, 0x1000);
  reset_chip(dev);

  write_fs(dev, SARMODE0, 0 | (0 * SARMODE0_SHADEN) | (1 * SARMODE0_INTMODE_READCLEAR) | (1 * SARMODE0_CWRE) | (IS_FS50(dev) ? SARMODE0_PRPWT_FS50_5 : SARMODE0_PRPWT_FS155_3) | (1 * SARMODE0_CALSUP_1) | (IS_FS50(dev) ? (0 | SARMODE0_RXVCS_32 | SARMODE0_ABRVCS_32 | SARMODE0_TXVCS_32) : (0 | SARMODE0_RXVCS_1k | SARMODE0_ABRVCS_1k | SARMODE0_TXVCS_1k)));

  to = 100;
  while (--to)
    {
      isr = read_fs(dev, ISR);

      if (isr & ISR_INIT_ERR)
        {
          printk(KERN_ERR "Error initializing the FS... \n");
          goto unmap;
        }
      if (isr & ISR_INIT)
        {
          fs_dprintk(FS_DEBUG_INIT, "Ha! Initialized OK!\n");
          break;
        }

      msleep(10);
    }
  if (!to)
    {
      printk(KERN_ERR "timeout initializing the FS... \n");
      goto unmap;
    }

  dev->channel_mask = 0x1f;
  dev->channo = 0;

  write_fs(dev, SARMODE1, 0 | (fs_keystream * SARMODE1_DEFHEC) | ((loopback == 1) * SARMODE1_TSTLP) | (1 * SARMODE1_DCRM) | (1 * SARMODE1_DCOAM) | (0 * SARMODE1_OAMCRC) | (0 * SARMODE1_DUMPE) | (0 * SARMODE1_GPLEN) | (0 * SARMODE1_GNAM) | (0 * SARMODE1_GVAS) | (0 * SARMODE1_GPAS) | (1 * SARMODE1_GPRI) | (0 * SARMODE1_PMS) | (0 * SARMODE1_GFCR) | (1 * SARMODE1_HECM2) | (1 * SARMODE1_HECM1) | (1 * SARMODE1_HECM0) | (1 << 12) | (0 * 0xff));

  write_fs(dev, TMCONF, 0x0000000f);
  write_fs(dev, CALPRESCALE, 0x01010101 * num);
  write_fs(dev, 0x80, 0x000F00E4);

  write_fs(dev, CELLOSCONF, 0 | (0 * CELLOSCONF_CEN) | (CELLOSCONF_SC1) | (0x80 * CELLOSCONF_COBS) | (num * CELLOSCONF_COPK) | (num * CELLOSCONF_COST));

  write_fs(dev, CELLOSCONF_COST, 0x0B809191);
  if (IS_FS50(dev))
    {
      write_fs(dev, RAS0, RAS0_DCD_XHLT);
      dev->atm_dev->ci_range.vpi_bits = 12;
      dev->atm_dev->ci_range.vci_bits = 16;
      dev->nchannels = FS50_NR_CHANNELS;
    }
  else
    {
      write_fs(dev, RAS0, RAS0_DCD_XHLT | (((1 << FS155_VPI_BITS) - 1) * RAS0_VPSEL) | (((1 << FS155_VCI_BITS) - 1) * RAS0_VCSEL));

      dev->atm_dev->ci_range.vpi_bits = FS155_VPI_BITS;
      dev->atm_dev->ci_range.vci_bits = FS155_VCI_BITS;

      write_fs(dev, RAC, 0);

      write_fs(dev, RAM, (1 << (28 - FS155_VPI_BITS - FS155_VCI_BITS)) - 1);
      dev->nchannels = FS155_NR_CHANNELS;
    }
  dev->atm_vccs = kcalloc(dev->nchannels, sizeof(struct atm_vcc *), GFP_KERNEL);
  fs_dprintk(FS_DEBUG_ALLOC, "Alloc atmvccs: %p(%zd)\n", dev->atm_vccs, dev->nchannels * sizeof(struct atm_vcc *));
  if (!dev->atm_vccs)
    {
      printk(KERN_WARNING "Couldn't allocate memory for VCC buffers. Woops!\n");

      goto unmap;
    }
  dev->tx_inuse = kzalloc(dev->nchannels / 8, GFP_KERNEL);
  fs_dprintk(FS_DEBUG_ALLOC, "Alloc tx_inuse: %p(%d)\n", dev->atm_vccs, dev->nchannels / 8);
  if (!dev->tx_inuse)
    {
      printk(KERN_WARNING "Couldn't allocate memory for tx_inuse bits!\n");

      goto unmap;
    }

  write_fs(dev, DMAMR, DMAMR_TX_MODE_FULL);
  init_q(dev, &dev->hp_txq, TX_PQ(TXQ_HP), TXQ_NENTRIES, 0);
  init_q(dev, &dev->lp_txq, TX_PQ(TXQ_LP), TXQ_NENTRIES, 0);
  init_q(dev, &dev->tx_relq, TXB_RQ, TXQ_NENTRIES, 1);
  init_q(dev, &dev->st_q, ST_Q, TXQ_NENTRIES, 1);
  for (i = 0; i < FS_NR_FREE_POOLS; i++)
    {
      init_fp(dev, &dev->rx_fp[i], RXB_FP(i), rx_buf_sizes[i], rx_pool_sizes[i]);
      top_off_fp(dev, &dev->rx_fp[i], GFP_KERNEL);
    }
  for (i = 0; i < FS_NR_RX_QUEUES; i++)
    init_q(dev, &dev->rx_rq[i], RXB_RQ(i), RXRQ_NENTRIES, 1);
  dev->irq = pci_dev->irq;
  if (request_irq(dev->irq, fs_irq, IRQF_SHARED, "firestream", dev))
    {
      printk(KERN_WARNING "couldn't get irq %d for firestream.\n", pci_dev->irq);

      goto unmap;
    }
  fs_dprintk(FS_DEBUG_INIT, "Grabbed irq %d for dev at %p.\n", dev->irq, dev);

  write_fs(dev, IMR, 0 | ISR_RBRQ0_W | ISR_RBRQ1_W | ISR_RBRQ2_W | ISR_RBRQ3_W | ISR_TBRQ_W | ISR_CSQ_W);
  write_fs(dev, SARMODE0, 0 | (0 * SARMODE0_SHADEN) | (1 * SARMODE0_GINT) | (1 * SARMODE0_INTMODE_READCLEAR) | (0 * SARMODE0_CWRE) | (IS_FS50(dev) ? SARMODE0_PRPWT_FS50_5 : SARMODE0_PRPWT_FS155_3) | (1 * SARMODE0_CALSUP_1) | (IS_FS50(dev) ? (0 | SARMODE0_RXVCS_32 | SARMODE0_ABRVCS_32 | SARMODE0_TXVCS_32) : (0 | SARMODE0_RXVCS_1k | SARMODE0_ABRVCS_1k | SARMODE0_TXVCS_1k)) | (1 * SARMODE0_RUN));
  init_phy(dev, PHY_NTC_INIT);
  if (loopback == 2)
    {
      write_phy(dev, 0x39, 0x000e);
    }
#ifdef FS_POLL_FREQ
  setup_timer(&dev->timer, fs_poll, (unsigned long)dev);
  dev->timer.expires = jiffies + FS_POLL_FREQ;
  add_timer(&dev->timer);
#endif
  dev->atm_dev->dev_data = dev;

  func_exit();
  return 0;
unmap:
  iounmap(dev->base);
  return 1;
}