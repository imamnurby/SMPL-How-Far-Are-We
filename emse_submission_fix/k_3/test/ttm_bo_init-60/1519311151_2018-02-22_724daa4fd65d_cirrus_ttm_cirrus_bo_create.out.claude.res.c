int cirrus_bo_create(struct drm_device *dev, int size, int align, uint32_t flags, struct cirrus_bo **pcirrusbo)
{
  struct cirrus_device *cirrus = dev->dev_private;
  struct cirrus_bo *cirrusbo;
  size_t acc_size;
  int ret;
  cirrusbo = kzalloc(sizeof(struct cirrus_bo), GFP_KERNEL);
  if (!cirrusbo)
    return -ENOMEM;
  ret = drm_gem_object_init(dev, &cirrusbo->gem, size);
  if (ret)
    {
      kfree(cirrusbo);
      return ret;
    }
  cirrusbo->bo.bdev = &cirrus->ttm.bdev;
  cirrus_ttm_placement(cirrusbo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);
  acc_size = ttm_bo_dma_acc_size(&cirrus->ttm.bdev, size, sizeof(struct cirrus_bo));
  ret = ttm_bo_init(&cirrus->ttm.bdev, &cirrusbo->bo, size, ttm_bo_type_device, &cirrusbo->placement, align >> PAGE_SHIFT, false,
                    acc_size, NULL, NULL, cirrus_bo_ttm_destroy);
  if (ret)
    return ret;
  *pcirrusbo = cirrusbo;
  return 0;
}