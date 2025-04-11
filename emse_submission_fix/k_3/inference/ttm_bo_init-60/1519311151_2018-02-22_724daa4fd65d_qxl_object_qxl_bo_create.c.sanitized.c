int qxl_bo_create(struct qxl_device *qdev, unsigned long size, bool kernel, bool pinned, u32 domain, struct qxl_surface *surf, struct qxl_bo **bo_ptr)
{
  struct qxl_bo *bo;
  enum ttm_bo_type type;
  int r;
  if (kernel)
    type = ttm_bo_type_kernel;
  else
    type = ttm_bo_type_device;
  *bo_ptr = NULL;
  bo = kzalloc(sizeof(struct qxl_bo), GFP_KERNEL);
  if (bo == NULL)
    return -ENOMEM;
  size = roundup(size, PAGE_SIZE);
  r = drm_gem_object_init(&qdev->ddev, &bo->gem_base, size);
  if (unlikely(r))
    {
      kfree(bo);
      return r;
    }
  bo->type = domain;
  bo->pin_count = pinned ? 1 : 0;
  bo->surface_id = 0;
  INIT_LIST_HEAD(&bo->list);
  if (surf)
    bo->surf = *surf;
  qxl_ttm_placement_from_domain(bo, domain, pinned);
  r = ttm_bo_init(&qdev->mman.bdev, &bo->tbo, size, type, &bo->placement, 0, !kernel, NULL, size, NULL, NULL, &qxl_ttm_bo_destroy);
  if (unlikely(r != 0))
    {
      if (r != -ERESTARTSYS)
        dev_err(qdev->ddev.dev, "object_init failed for (%lu, 0x%08X)\n", size, domain);
      return r;
    }
  *bo_ptr = bo;
  return 0;
}