@@
expression bdev, bo, size, type, placement, align, sg, robj, destroy_fn;
@@
- ttm_bo_init(bdev, bo, size, type, placement, align, false, NULL, size, sg, robj, destroy_fn)
+ ttm_bo_init(bdev, bo, size, type, placement, align, false, size, sg, robj, destroy_fn)