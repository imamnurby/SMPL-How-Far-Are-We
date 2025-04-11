static int __init n2100_request_gpios(void)
{
  int ret;
  if (!machine_is_n2100())
    return 0;
  ret = gpio_request(N2100_HARDWARE_RESET, "reset");
  if (ret)
    pr_err("could not request reset GPIO\n");
  ret = gpio_request(N2100_POWER_BUTTON, "power");
  if (ret)
    pr_err("could not request power GPIO\n");
  else
    {
      ret = gpio_direction_input(N2100_POWER_BUTTON);
      if (ret)
        pr_err("could not set power GPIO as input\n");
    }

  init_timer(&power_button_poll_timer);
  power_button_poll_timer.function = power_button_poll;
  power_button_poll_timer.expires = jiffies + (HZ / 10);
  add_timer(&power_button_poll_timer);
  return 0;
}