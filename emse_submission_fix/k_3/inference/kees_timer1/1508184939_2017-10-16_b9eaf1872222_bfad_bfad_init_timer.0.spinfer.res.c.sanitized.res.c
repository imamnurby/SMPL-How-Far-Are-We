void bfad_init_timer(struct bfad_s *bfad)
{
  setup_timer(&bfad->hal_tmo, bfad_bfa_tmo, (unsigned long)bfad);
  mod_timer(&bfad->hal_tmo, jiffies + msecs_to_jiffies(BFA_TIMER_FREQ));
}