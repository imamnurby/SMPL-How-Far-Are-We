void pcibios_enable_timers(struct pci_channel *hose)
{
  if (hose->err_irq)
    {
      setup_timer(&hose->err_timer, pcibios_enable_err, (unsigned long)hose);
    }
  if (hose->serr_irq)
    {
      setup_timer(&hose->serr_timer, pcibios_enable_serr, (unsigned long)hose);
    }
}