void bfad_init_timer(struct bfad_s *bfad)
{
  init_timer(&bfad->hal_tmo);
  bfad->hal_tmo.function = bfad_bfa_tmo;
  bfad->hal_tmo.data = (unsigned long)bfad;
  mod_timer(&bfad->hal_tmo, jiffies + msecs_to_jiffies(BFA_TIMER_FREQ));
}