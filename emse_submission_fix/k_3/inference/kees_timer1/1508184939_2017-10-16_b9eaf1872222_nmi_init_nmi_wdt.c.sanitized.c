static int __init init_nmi_wdt(void)
{
  nmi_wdt_set_timeout(timeout);
  nmi_wdt_start();
  nmi_active = true;
  init_timer(&ntimer);
  ntimer.function = nmi_wdt_timer;
  ntimer.expires = jiffies + NMI_CHECK_TIMEOUT;
  add_timer(&ntimer);
  pr_info("nmi_wdt: initialized: timeout=%d sec\n", timeout);
  return 0;
}