void mthca_start_catas_poll(struct mthca_dev *dev)
{
  phys_addr_t addr;
  init_timer(&dev->catas_err.timer);
  dev->catas_err.map = NULL;
  addr = pci_resource_start(dev->pdev, 0) + ((pci_resource_len(dev->pdev, 0) - 1) & dev->catas_err.addr);
  dev->catas_err.map = ioremap(addr, dev->catas_err.size * 4);
  if (!dev->catas_err.map)
    {
      mthca_warn(dev,
                 "couldn't map catastrophic error region "
                 "at 0x%llx/0x%x\n",
                 (unsigned long long)addr,
                 dev->catas_err.size * 4);
      return;
    }
  dev->catas_err.timer.data = (unsigned long)dev;
  dev->catas_err.timer.function = poll_catas;
  dev->catas_err.timer.expires = jiffies + MTHCA_CATAS_POLL_INTERVAL;
  INIT_LIST_HEAD(&dev->catas_err.list);
  add_timer(&dev->catas_err.timer);
}