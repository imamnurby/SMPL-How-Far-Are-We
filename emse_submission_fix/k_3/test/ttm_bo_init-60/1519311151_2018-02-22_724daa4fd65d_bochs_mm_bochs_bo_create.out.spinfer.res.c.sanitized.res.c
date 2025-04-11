static int bochs_bo_create(struct drm_device *dev, int size, int align, uint32_t flags, struct bochs_bo **pbochsbo)
{
  struct bochs_device *bochs = dev->dev_private;
  struct bochs_bo *bochsbo;
  size_t acc_size;
  int ret;
  bochsbo = kzalloc(sizeof(struct bochs_bo), GFP_KERNEL);
  if (!bochsbo)
    return -ENOMEM;
  ret = drm_gem_object_init(dev, &bochsbo->gem, size);
  if (ret)
    {
      kfree(bochsbo);
      return ret;
    }
  bochsbo->bo.bdev = &bochs->ttm.bdev;
  bochsbo->bo.bdev->dev_mapping = dev->anon_inode->i_mapping;
  bochs_ttm_placement(bochsbo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);
  acc_size = ttm_bo_dma_acc_size(&bochs->ttm.bdev, size, sizeof(struct bochs_bo));
  ret = ttm_bo_init(&bochs->ttm.bdev, &bochsbo->bo, size, ttm_bo_type_device, &bochsbo->placement, align >> PAGE_SHIFT, false, acc_size, NULL, NULL, ast_bo_ttm_destroy);
  if (ret)
    return ret;
  *pbochsbo = bochsbo;
  return 0;
}