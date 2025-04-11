struct Scsi_Host *__init ncr_attach(struct scsi_host_template *tpnt, int unit, struct ncr_device *device)
{
  struct host_data *host_data;
  struct ncb *np = NULL;
  struct Scsi_Host *instance = NULL;
  u_long flags = 0;
  int i;
  if (!tpnt->name)
    tpnt->name = SCSI_NCR_DRIVER_NAME;
  if (!tpnt->shost_attrs)
    tpnt->shost_attrs = ncr53c8xx_host_attrs;
  tpnt->queuecommand = ncr53c8xx_queue_command;
  tpnt->slave_configure = ncr53c8xx_slave_configure;
  tpnt->slave_alloc = ncr53c8xx_slave_alloc;
  tpnt->eh_bus_reset_handler = ncr53c8xx_bus_reset;
  tpnt->can_queue = SCSI_NCR_CAN_QUEUE;
  tpnt->this_id = 7;
  tpnt->sg_tablesize = SCSI_NCR_SG_TABLESIZE;
  tpnt->cmd_per_lun = SCSI_NCR_CMD_PER_LUN;
  tpnt->use_clustering = ENABLE_CLUSTERING;
  if (device->differential)
    driver_setup.diff_support = device->differential;
  printk(KERN_INFO "ncr53c720-%d: rev 0x%x irq %d\n", unit, device->chip.revision_id, device->slot.irq);
  instance = scsi_host_alloc(tpnt, sizeof(*host_data));
  if (!instance)
    goto attach_error;
  host_data = (struct host_data *)instance->hostdata;
  np = __m_calloc_dma(device->dev, sizeof(struct ncb), "NCB");
  if (!np)
    goto attach_error;
  spin_lock_init(&np->smp_lock);
  np->dev = device->dev;
  np->p_ncb = vtobus(np);
  host_data->ncb = np;
  np->ccb = m_calloc_dma(sizeof(struct ccb), "CCB");
  if (!np->ccb)
    goto attach_error;

  np->unit = unit;
  np->verbose = driver_setup.verbose;
  sprintf(np->inst_name, "ncr53c720-%d", np->unit);
  np->revision_id = device->chip.revision_id;
  np->features = device->chip.features;
  np->clock_divn = device->chip.nr_divisor;
  np->maxoffs = device->chip.offset_max;
  np->maxburst = device->chip.burst_max;
  np->myaddr = device->host_id;

  np->script0 = m_calloc_dma(sizeof(struct script), "SCRIPT");
  if (!np->script0)
    goto attach_error;
  np->scripth0 = m_calloc_dma(sizeof(struct scripth), "SCRIPTH");
  if (!np->scripth0)
    goto attach_error;
  setup_timer(&np->timer, ncr53c8xx_timeout, (unsigned long)np);

  np->paddr = device->slot.base;
  np->paddr2 = (np->features & FE_RAM) ? device->slot.base_2 : 0;
  if (device->slot.base_v)
    np->vaddr = device->slot.base_v;
  else
    np->vaddr = ioremap(device->slot.base_c, 128);
  if (!np->vaddr)
    {
      printk(KERN_ERR "%s: can't map memory mapped IO region\n", ncr_name(np));
      goto attach_error;
    }
  else
    {
      if (bootverbose > 1)
        printk(KERN_INFO "%s: using memory mapped IO at virtual address 0x%lx\n", ncr_name(np), (u_long)np->vaddr);
    }

  np->reg = (struct ncr_reg __iomem *)np->vaddr;

  ncr_prepare_setting(np);
  if (np->paddr2 && sizeof(struct script) > 4096)
    {
      np->paddr2 = 0;
      printk(KERN_WARNING "%s: script too large, NOT using on chip RAM.\n", ncr_name(np));
    }
  instance->max_channel = 0;
  instance->this_id = np->myaddr;
  instance->max_id = np->maxwide ? 16 : 8;
  instance->max_lun = SCSI_NCR_MAX_LUN;
  instance->base = (unsigned long)np->reg;
  instance->irq = device->slot.irq;
  instance->unique_id = device->slot.base;
  instance->dma_channel = 0;
  instance->cmd_per_lun = MAX_TAGS;
  instance->can_queue = (MAX_START - 4);

  BUG_ON(!ncr53c8xx_transport_template);
  instance->transportt = ncr53c8xx_transport_template;

  ncr_script_fill(&script0, &scripth0);
  np->scripth = np->scripth0;
  np->p_scripth = vtobus(np->scripth);
  np->p_script = (np->paddr2) ? np->paddr2 : vtobus(np->script0);
  ncr_script_copy_and_bind(np, (ncrcmd *)&script0, (ncrcmd *)np->script0, sizeof(struct script));
  ncr_script_copy_and_bind(np, (ncrcmd *)&scripth0, (ncrcmd *)np->scripth0, sizeof(struct scripth));
  np->ccb->p_ccb = vtobus(np->ccb);

  if (np->features & FE_LED0)
    {
      np->script0->idle[0] = cpu_to_scr(SCR_REG_REG(gpreg, SCR_OR, 0x01));
      np->script0->reselected[0] = cpu_to_scr(SCR_REG_REG(gpreg, SCR_AND, 0xfe));
      np->script0->start[0] = cpu_to_scr(SCR_REG_REG(gpreg, SCR_AND, 0xfe));
    }

  for (i = 0; i < 4; i++)
    {
      np->jump_tcb[i].l_cmd = cpu_to_scr((SCR_JUMP ^ IFTRUE(MASK(i, 3))));
      np->jump_tcb[i].l_paddr = cpu_to_scr(NCB_SCRIPTH_PHYS(np, bad_target));
    }
  ncr_chip_reset(np, 100);

  if (ncr_snooptest(np))
    {
      printk(KERN_ERR "CACHE INCORRECTLY CONFIGURED.\n");
      goto attach_error;
    }

  np->irq = device->slot.irq;

  ncr_init_ccb(np, np->ccb);

  spin_lock_irqsave(&np->smp_lock, flags);
  if (ncr_reset_scsi_bus(np, 0, driver_setup.settle_delay) != 0)
    {
      printk(KERN_ERR "%s: FATAL ERROR: CHECK SCSI BUS - CABLES, TERMINATION, DEVICE POWER etc.!\n", ncr_name(np));
      spin_unlock_irqrestore(&np->smp_lock, flags);
      goto attach_error;
    }
  ncr_exception(np);
  np->disc = 1;

  if (driver_setup.settle_delay > 2)
    {
      printk(KERN_INFO "%s: waiting %d seconds for scsi devices to settle...\n", ncr_name(np), driver_setup.settle_delay);
      mdelay(1000 * driver_setup.settle_delay);
    }

  np->lasttime = 0;
  ncr_timeout(np);

#ifdef SCSI_NCR_ALWAYS_SIMPLE_TAG
  np->order = SIMPLE_QUEUE_TAG;
#endif
  spin_unlock_irqrestore(&np->smp_lock, flags);
  return instance;
attach_error:
  if (!instance)
    return NULL;
  printk(KERN_INFO "%s: detaching...\n", ncr_name(np));
  if (!np)
    goto unregister;
  if (np->scripth0)
    m_free_dma(np->scripth0, sizeof(struct scripth), "SCRIPTH");
  if (np->script0)
    m_free_dma(np->script0, sizeof(struct script), "SCRIPT");
  if (np->ccb)
    m_free_dma(np->ccb, sizeof(struct ccb), "CCB");
  m_free_dma(np, sizeof(struct ncb), "NCB");
  host_data->ncb = NULL;
unregister:
  scsi_host_put(instance);
  return NULL;
}