static inline void lanai_timed_poll_start(struct lanai_dev *lanai)
{
	init_timer(&lanai->timer);
	lanai->timer.expires = jiffies + LANAI_POLL_PERIOD;
	lanai->timer.data = (unsigned long) lanai;
	lanai->timer.function = lanai_timed_poll;
	add_timer(&lanai->timer);
}