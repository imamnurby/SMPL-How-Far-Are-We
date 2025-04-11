int ast_bo_create(struct drm_device *dev, int size, int align,
		  uint32_t flags, struct ast_bo **pastbo)
{
	struct ast_private *ast = dev->dev_private;
	struct ast_bo *astbo;
	size_t acc_size;
	int ret;
	astbo = kzalloc(sizeof(struct ast_bo), GFP_KERNEL);
	if (!astbo)
		return -ENOMEM;
	ret = drm_gem_object_init(dev, &astbo->gem, size);
	if (ret)
		goto error;
	astbo->bo.bdev = &ast->ttm.bdev;
	ast_ttm_placement(astbo, TTM_PL_FLAG_VRAM | TTM_PL_FLAG_SYSTEM);
	acc_size = ttm_bo_dma_acc_size(&ast->ttm.bdev, size,
				       sizeof(struct ast_bo));
	ret = ttm_bo_init(&ast->ttm.bdev, &astbo->bo, size,
			  ttm_bo_type_device, &astbo->placement,
			  align >> PAGE_SHIFT, false, acc_size,
			  NULL, NULL, ast_bo_ttm_destroy);
	if (ret)
		goto error;
	*pastbo = astbo;
	return 0;
error:
	kfree(astbo);
	return ret;
}