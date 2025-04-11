static int switch_drv_probe(struct platform_device *pdev)
{
  struct push_switch_platform_info *psw_info;
  struct push_switch *psw;
  int ret, irq;
  psw = kzalloc(sizeof(struct push_switch), GFP_KERNEL);
  if (unlikely(!psw))
    return -ENOMEM;
  irq = platform_get_irq(pdev, 0);
  if (unlikely(irq < 0))
    {
      ret = -ENODEV;
      goto err;
    }
  psw_info = pdev->dev.platform_data;
  BUG_ON(!psw_info);
  ret = request_irq(irq, psw_info->irq_handler, psw_info->irq_flags, psw_info->name ? psw_info->name : DRV_NAME, pdev);
  if (unlikely(ret < 0))
    goto err;
  if (psw_info->name)
    {
      ret = device_create_file(&pdev->dev, &dev_attr_switch);
      if (unlikely(ret))
        {
          dev_err(&pdev->dev, "Failed creating device attrs\n");
          ret = -EINVAL;
          goto err_irq;
        }
    }
  INIT_WORK(&psw->work, switch_work_handler);
  init_timer(&psw->debounce);
  psw->debounce.function = switch_timer;
  psw->debounce.data = (unsigned long)psw;

  psw->pdev = pdev;
  platform_set_drvdata(pdev, psw);
  return 0;
err_irq:
  free_irq(irq, pdev);
err:
  kfree(psw);
  return ret;
}