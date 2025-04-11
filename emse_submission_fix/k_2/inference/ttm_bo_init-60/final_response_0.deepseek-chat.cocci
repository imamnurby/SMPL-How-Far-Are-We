@@
expression dev, bo, size, type, placement, align, acc_size, sg, robj, destroy;
@@
- ttm_bo_init(dev, bo, size, type, placement, align, false, NULL, acc_size, sg, robj, destroy)
+ ttm_bo_init(dev, bo, size, type, placement, align, false, acc_size, sg, robj, destroy)