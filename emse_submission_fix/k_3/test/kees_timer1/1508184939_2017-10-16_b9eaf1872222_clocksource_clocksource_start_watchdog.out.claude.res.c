static inline void clocksource_start_watchdog(void)
{
  if (watchdog_running || !watchdog || list_empty(&watchdog_list))
    return;
  setup_timer(&watchdog_timer, clocksource_watchdog, 0UL);
  watchdog_timer.expires = jiffies + WATCHDOG_INTERVAL;
  add_timer_on(&watchdog_timer, cpumask_first(cpu_online_mask));
  watchdog_running = 1;
}