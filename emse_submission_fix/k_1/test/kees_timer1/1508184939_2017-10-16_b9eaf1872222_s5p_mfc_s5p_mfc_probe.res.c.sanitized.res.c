static int s5p_mfc_probe(struct platform_device *pdev)
{
  struct s5p_mfc_dev *dev;
  struct video_device *vfd;
  struct resource *res;
  int ret;
  pr_debug("%s++\n", __func__);
  dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
  if (!dev)
    return -ENOMEM;
  spin_lock_init(&dev->irqlock);
  spin_lock_init(&dev->condlock);
  dev->plat_dev = pdev;
  if (!dev->plat_dev)
    {
      dev_err(&pdev->dev, "No platform data specified\n");
      return -ENODEV;
    }
  dev->variant = of_device_get_match_data(&pdev->dev);
  res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  dev->regs_base = devm_ioremap_resource(&pdev->dev, res);
  if (IS_ERR(dev->regs_base))
    return PTR_ERR(dev->regs_base);
  res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
  if (!res)
    {
      dev_err(&pdev->dev, "failed to get irq resource\n");
      return -ENOENT;
    }
  dev->irq = res->start;
  ret = devm_request_irq(&pdev->dev, dev->irq, s5p_mfc_irq, 0, pdev->name, dev);
  if (ret)
    {
      dev_err(&pdev->dev, "Failed to install irq (%d)\n", ret);
      return ret;
    }
  ret = s5p_mfc_configure_dma_memory(dev);
  if (ret < 0)
    {
      dev_err(&pdev->dev, "failed to configure DMA memory\n");
      return ret;
    }
  ret = s5p_mfc_init_pm(dev);
  if (ret < 0)
    {
      dev_err(&pdev->dev, "failed to get mfc clock source\n");
      goto err_dma;
    }
  mutex_init(&dev->mfc_mutex);
  init_waitqueue_head(&dev->queue);
  dev->hw_lock = 0;
  INIT_WORK(&dev->watchdog_work, s5p_mfc_watchdog_worker);
  atomic_set(&dev->watchdog_cnt, 0);
  setup_timer(&dev->watchdog_timer, s5p_mfc_watchdog, (unsigned long)dev);
  ret = v4l2_device_register(&pdev->dev, &dev->v4l2_dev);
  if (ret)
    goto err_v4l2_dev_reg;

  vfd = video_device_alloc();
  if (!vfd)
    {
      v4l2_err(&dev->v4l2_dev, "Failed to allocate video device\n");
      ret = -ENOMEM;
      goto err_dec_alloc;
    }
  vfd->fops = &s5p_mfc_fops;
  vfd->ioctl_ops = get_dec_v4l2_ioctl_ops();
  vfd->release = video_device_release;
  vfd->lock = &dev->mfc_mutex;
  vfd->v4l2_dev = &dev->v4l2_dev;
  vfd->vfl_dir = VFL_DIR_M2M;
  snprintf(vfd->name, sizeof(vfd->name), "%s", S5P_MFC_DEC_NAME);
  dev->vfd_dec = vfd;
  video_set_drvdata(vfd, dev);

  vfd = video_device_alloc();
  if (!vfd)
    {
      v4l2_err(&dev->v4l2_dev, "Failed to allocate video device\n");
      ret = -ENOMEM;
      goto err_enc_alloc;
    }
  vfd->fops = &s5p_mfc_fops;
  vfd->ioctl_ops = get_enc_v4l2_ioctl_ops();
  vfd->release = video_device_release;
  vfd->lock = &dev->mfc_mutex;
  vfd->v4l2_dev = &dev->v4l2_dev;
  vfd->vfl_dir = VFL_DIR_M2M;
  snprintf(vfd->name, sizeof(vfd->name), "%s", S5P_MFC_ENC_NAME);
  dev->vfd_enc = vfd;
  video_set_drvdata(vfd, dev);
  platform_set_drvdata(pdev, dev);

  s5p_mfc_init_hw_ops(dev);
  s5p_mfc_init_hw_cmds(dev);
  s5p_mfc_init_regs(dev);

  ret = video_register_device(dev->vfd_dec, VFL_TYPE_GRABBER, 0);
  if (ret)
    {
      v4l2_err(&dev->v4l2_dev, "Failed to register video device\n");
      goto err_dec_reg;
    }
  v4l2_info(&dev->v4l2_dev, "decoder registered as /dev/video%d\n", dev->vfd_dec->num);
  ret = video_register_device(dev->vfd_enc, VFL_TYPE_GRABBER, 0);
  if (ret)
    {
      v4l2_err(&dev->v4l2_dev, "Failed to register video device\n");
      goto err_enc_reg;
    }
  v4l2_info(&dev->v4l2_dev, "encoder registered as /dev/video%d\n", dev->vfd_enc->num);
  pr_debug("%s--\n", __func__);
  return 0;

err_enc_reg:
  video_unregister_device(dev->vfd_dec);
err_dec_reg:
  video_device_release(dev->vfd_enc);
err_enc_alloc:
  video_device_release(dev->vfd_dec);
err_dec_alloc:
  v4l2_device_unregister(&dev->v4l2_dev);
err_v4l2_dev_reg:
  s5p_mfc_final_pm(dev);
err_dma:
  s5p_mfc_unconfigure_dma_memory(dev);
  pr_debug("%s-- with error\n", __func__);
  return ret;
}