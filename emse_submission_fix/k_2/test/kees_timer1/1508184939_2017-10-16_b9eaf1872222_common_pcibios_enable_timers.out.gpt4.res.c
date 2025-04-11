void pcibios_enable_timers(struct pci_channel *hose)
{
  if (hose->err_irq)
    {
      init_timer(&hose->err_timer);
      hose->err_timer.data = (unsigned long)hose;
      hose->err_timer.function = pcibios_enable_err;
    }
  if (hose->serr_irq)
    {
      init_timer(&hose->serr_timer);
      hose->serr_timer.data = (unsigned long)hose;
      hose->serr_timer.function = pcibios_enable_serr;
    }
}