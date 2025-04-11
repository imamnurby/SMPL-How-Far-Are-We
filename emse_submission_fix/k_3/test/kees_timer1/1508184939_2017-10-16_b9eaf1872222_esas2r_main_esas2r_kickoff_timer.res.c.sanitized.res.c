void esas2r_kickoff_timer(struct esas2r_adapter *a)
{
  setup_timer(&a->timer, esas2r_timer_callback, (unsigned long)a);
  a->timer.expires = jiffies + msecs_to_jiffies(100);
  add_timer(&a->timer);
}