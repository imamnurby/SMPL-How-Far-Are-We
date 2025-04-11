int vbox_bo_create(struct drm_device *dev, int size, int align, u32 flags, struct vbox_bo **pvboxbo)
{
  struct vbox_private *vbox = dev->dev_private;
  struct vbox_bo *vboxbo;
  size_t acc_size;
  int ret;
  vboxbo = kzalloc(sizeof(*vboxbo), GFP_KERNEL);
  if (!vboxbo)
    return -ENOMEM;
  ret = drm_gem_object_init(dev, &vboxbo->gem, size);
  if (ret)
    goto err_free_vboxbo;
  vboxbo->bo.bdev = &vbox->ttm.bdev;
  vbox_ttm_placement(vboxbo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);
  acc_size = ttm_bo_dma_acc_size(&vbox->ttm.bdev, size, sizeof(struct vbox_bo));
  ret = ttm_bo_init(&vbox->ttm.bdev, &vboxbo->bo, size, ttm_bo_type_device, &vboxbo->placement, align >> PAGE_SHIFT, false, acc_size, NULL, NULL, vbox_bo_ttm_destroy);
  if (ret)
    goto err_free_vboxbo;
  *pvboxbo = vboxbo;
  return 0;
err_free_vboxbo:
  kfree(vboxbo);
  return ret;
}