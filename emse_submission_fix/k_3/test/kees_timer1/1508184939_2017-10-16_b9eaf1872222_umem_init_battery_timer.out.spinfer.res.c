static void init_battery_timer(void)
{
  setup_timer(&battery_timer, check_all_batteries, 0UL);
  battery_timer.expires = jiffies + (HZ * 60);
  add_timer(&battery_timer);
}