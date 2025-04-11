static inline void lanai_timed_poll_start(struct lanai_dev *lanai)
{
  setup_timer(&lanai->timer, lanai_timed_poll, (unsigned long)lanai);
  lanai->timer.expires = jiffies + LANAI_POLL_PERIOD;
  add_timer(&lanai->timer);
}