static struct Scsi_Host *sym_attach(struct scsi_host_template *tpnt, int unit, struct sym_device *dev)
{
  struct sym_data *sym_data;
  struct sym_hcb *np = NULL;
  struct Scsi_Host *shost = NULL;
  struct pci_dev *pdev = dev->pdev;
  unsigned long flags;
  struct sym_fw *fw;
  int do_free_irq = 0;
  printk(KERN_INFO "sym%d: <%s> rev 0x%x at pci %s irq %u\n", unit, dev->chip.name, pdev->revision, pci_name(pdev), pdev->irq);

  fw = sym_find_firmware(&dev->chip);
  if (!fw)
    goto attach_failed;
  shost = scsi_host_alloc(tpnt, sizeof(*sym_data));
  if (!shost)
    goto attach_failed;
  sym_data = shost_priv(shost);

  np = __sym_calloc_dma(&pdev->dev, sizeof(*np), "HCB");
  if (!np)
    goto attach_failed;
  np->bus_dmat = &pdev->dev;
  sym_data->ncb = np;
  sym_data->pdev = pdev;
  np->s.host = shost;
  pci_set_drvdata(pdev, shost);

  np->hcb_ba = vtobus(np);
  np->verbose = sym_driver_setup.verbose;
  np->s.unit = unit;
  np->features = dev->chip.features;
  np->clock_divn = dev->chip.nr_divisor;
  np->maxoffs = dev->chip.offset_max;
  np->maxburst = dev->chip.burst_max;
  np->myaddr = dev->host_id;
  np->mmio_ba = (u32)dev->mmio_base;
  np->ram_ba = (u32)dev->ram_base;
  np->s.ioaddr = dev->s.ioaddr;
  np->s.ramaddr = dev->s.ramaddr;

  strlcpy(np->s.chip_name, dev->chip.name, sizeof(np->s.chip_name));
  sprintf(np->s.inst_name, "sym%d", np->s.unit);
  if ((SYM_CONF_DMA_ADDRESSING_MODE > 0) && (np->features & FE_DAC) && !pci_set_dma_mask(pdev, DMA_DAC_MASK))
    {
      set_dac(np);
    }
  else if (pci_set_dma_mask(pdev, DMA_BIT_MASK(32)))
    {
      printf_warning("%s: No suitable DMA available\n", sym_name(np));
      goto attach_failed;
    }
  if (sym_hcb_attach(shost, fw, dev->nvram))
    goto attach_failed;

  if (request_irq(pdev->irq, sym53c8xx_intr, IRQF_SHARED, NAME53C8XX, shost))
    {
      printf_err("%s: request irq %u failure\n", sym_name(np), pdev->irq);
      goto attach_failed;
    }
  do_free_irq = 1;

  spin_lock_irqsave(shost->host_lock, flags);
  if (sym_reset_scsi_bus(np, 0))
    goto reset_failed;

  sym_start_up(shost, 1);

  setup_timer(&np->s.timer, sym53c8xx_timer, (unsigned long)np);
  np->s.lasttime = 0;
  sym_timer(np);

  shost->max_channel = 0;
  shost->this_id = np->myaddr;
  shost->max_id = np->maxwide ? 16 : 8;
  shost->max_lun = SYM_CONF_MAX_LUN;
  shost->unique_id = pci_resource_start(pdev, 0);
  shost->cmd_per_lun = SYM_CONF_MAX_TAG;
  shost->can_queue = (SYM_CONF_MAX_START - 2);
  shost->sg_tablesize = SYM_CONF_MAX_SG;
  shost->max_cmd_len = 16;
  BUG_ON(sym2_transport_template == NULL);
  shost->transportt = sym2_transport_template;

  if (pdev->device == PCI_DEVICE_ID_NCR_53C896 && pdev->revision < 2)
    shost->dma_boundary = 0xFFFFFF;
  spin_unlock_irqrestore(shost->host_lock, flags);
  return shost;
reset_failed:
  printf_err("%s: FATAL ERROR: CHECK SCSI BUS - CABLES, "
             "TERMINATION, DEVICE POWER etc.!\n",
             sym_name(np));
  spin_unlock_irqrestore(shost->host_lock, flags);
attach_failed:
  printf_info("sym%d: giving up ...\n", unit);
  if (np)
    sym_free_resources(np, pdev, do_free_irq);
  else
    sym_iounmap_device(dev);
  if (shost)
    scsi_host_put(shost);
  return NULL;
}