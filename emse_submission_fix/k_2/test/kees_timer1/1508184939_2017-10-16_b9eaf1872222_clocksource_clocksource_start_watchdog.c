static inline void clocksource_start_watchdog(void)
{
	if (watchdog_running || !watchdog || list_empty(&watchdog_list))
		return;
	init_timer(&watchdog_timer);
	watchdog_timer.function = clocksource_watchdog;
	watchdog_timer.expires = jiffies + WATCHDOG_INTERVAL;
	add_timer_on(&watchdog_timer, cpumask_first(cpu_online_mask));
	watchdog_running = 1;
}