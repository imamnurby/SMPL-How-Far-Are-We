static void init_battery_timer(void)
{
	init_timer(&battery_timer);
	battery_timer.function = check_all_batteries;
	battery_timer.expires = jiffies + (HZ * 60);
	add_timer(&battery_timer);
}