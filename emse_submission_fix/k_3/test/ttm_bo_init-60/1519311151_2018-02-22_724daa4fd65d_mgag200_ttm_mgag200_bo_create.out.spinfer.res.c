int mgag200_bo_create(struct drm_device *dev, int size, int align, uint32_t flags, struct mgag200_bo **pmgabo)
{
  struct mga_device *mdev = dev->dev_private;
  struct mgag200_bo *mgabo;
  size_t acc_size;
  int ret;
  mgabo = kzalloc(sizeof(struct mgag200_bo), GFP_KERNEL);
  if (!mgabo)
    return -ENOMEM;
  ret = drm_gem_object_init(dev, &mgabo->gem, size);
  if (ret)
    {
      kfree(mgabo);
      return ret;
    }
  mgabo->bo.bdev = &mdev->ttm.bdev;
  mgag200_ttm_placement(mgabo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);
  acc_size = ttm_bo_dma_acc_size(&mdev->ttm.bdev, size, sizeof(struct mgag200_bo));
  ret = ttm_bo_init(&mdev->ttm.bdev, &mgabo->bo, size, ttm_bo_type_device,
                    &mgabo->placement, align >> PAGE_SHIFT, false, acc_size,
                    NULL, NULL, ast_bo_ttm_destroy);
  if (ret)
    return ret;
  *pmgabo = mgabo;
  return 0;
}