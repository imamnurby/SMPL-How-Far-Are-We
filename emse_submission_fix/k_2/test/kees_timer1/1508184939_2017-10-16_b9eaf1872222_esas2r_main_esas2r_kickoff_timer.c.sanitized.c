void esas2r_kickoff_timer(struct esas2r_adapter *a)
{
  init_timer(&a->timer);
  a->timer.function = esas2r_timer_callback;
  a->timer.data = (unsigned long)a;
  a->timer.expires = jiffies + msecs_to_jiffies(100);
  add_timer(&a->timer);
}