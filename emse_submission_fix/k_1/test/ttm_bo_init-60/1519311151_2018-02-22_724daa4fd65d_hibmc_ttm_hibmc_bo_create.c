int hibmc_bo_create(struct drm_device *dev, int size, int align, u32 flags, struct hibmc_bo **phibmcbo) {
  struct hibmc_drm_private *hibmc = dev->dev_private;
  struct hibmc_bo *hibmcbo;
  size_t acc_size;
  int ret;
  hibmcbo = kzalloc(sizeof(*hibmcbo), GFP_KERNEL);
  if (!hibmcbo) {
    DRM_ERROR("failed to allocate hibmcbo\n");
    return -ENOMEM;
  }
  ret = drm_gem_object_init(dev, &hibmcbo->gem, size);
  if (ret) {
    DRM_ERROR("failed to initialize drm gem object: %d\n", ret);
    kfree(hibmcbo);
    return ret;
  }
  hibmcbo->bo.bdev = &hibmc->bdev;
  hibmc_ttm_placement(hibmcbo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);
  acc_size = ttm_bo_dma_acc_size(&hibmc->bdev, size, sizeof(struct hibmc_bo));
  ret = ttm_bo_init(&hibmc->bdev, &hibmcbo->bo, size, ttm_bo_type_device, &hibmcbo->placement, align >> PAGE_SHIFT, false, NULL, acc_size, NULL, NULL, hibmc_bo_ttm_destroy);
  if (ret) {
    hibmc_bo_unref(&hibmcbo);
    DRM_ERROR("failed to initialize ttm_bo: %d\n", ret);
    return ret;
  }
  *phibmcbo = hibmcbo;
  return 0;
}