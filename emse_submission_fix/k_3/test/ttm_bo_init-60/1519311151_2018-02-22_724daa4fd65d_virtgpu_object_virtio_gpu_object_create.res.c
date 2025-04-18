int virtio_gpu_object_create(struct virtio_gpu_device *vgdev, unsigned long size, bool kernel, bool pinned, struct virtio_gpu_object **bo_ptr) {
  struct virtio_gpu_object *bo;
  enum ttm_bo_type type;
  size_t acc_size;
  int ret;
  if (kernel)
    type = ttm_bo_type_kernel;
  else
    type = ttm_bo_type_device;
  *bo_ptr = NULL;
  acc_size = ttm_bo_dma_acc_size(&vgdev->mman.bdev, size, sizeof(struct virtio_gpu_object));
  bo = kzalloc(sizeof(struct virtio_gpu_object), GFP_KERNEL);
  if (bo == NULL) return -ENOMEM;
  size = roundup(size, PAGE_SIZE);
  ret = drm_gem_object_init(vgdev->ddev, &bo->gem_base, size);
  if (ret != 0) {
    kfree(bo);
    return ret;
  }
  bo->dumb = false;
  virtio_gpu_init_ttm_placement(bo, pinned);
  ret = ttm_bo_init(&vgdev->mman.bdev, &bo->tbo, size, type, &bo->placement, 0, !kernel, acc_size, NULL, NULL, &virtio_gpu_ttm_bo_destroy);
  /* ttm_bo_init failure will call the destroy */
  if (ret != 0) return ret;
  *bo_ptr = bo;
  return 0;
}