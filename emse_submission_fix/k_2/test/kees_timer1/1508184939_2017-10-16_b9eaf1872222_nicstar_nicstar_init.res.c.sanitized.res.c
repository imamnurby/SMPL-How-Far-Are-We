static int __init nicstar_init(void)
{
  unsigned error = 0;
  XPRINTK("nicstar: nicstar_init() called.\n");
  error = pci_register_driver(&nicstar_driver);
  TXPRINTK("nicstar: TX debug enabled.\n");
  RXPRINTK("nicstar: RX debug enabled.\n");
  PRINTK("nicstar: General debug enabled.\n");
#ifdef PHY_LOOPBACK
  printk("nicstar: using PHY loopback.\n");
#endif
  XPRINTK("nicstar: nicstar_init() returned.\n");
  if (!error)
    {
      setup_timer(&ns_timer, ns_poll, 0UL);
      ns_timer.expires = jiffies + NS_POLL_PERIOD;
      add_timer(&ns_timer);
    }
  return error;
}