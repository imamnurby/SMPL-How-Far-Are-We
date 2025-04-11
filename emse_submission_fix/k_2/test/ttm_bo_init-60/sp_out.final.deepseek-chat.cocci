@@
expression dev, bo, size, type, placement, align, acc_size, sg, robj, destroy;
@@
- ttm_bo_init(dev, bo, size, type, placement, align >> PAGE_SHIFT, false, NULL, acc_size, sg, robj, destroy)
+ ttm_bo_init(dev, bo, size, type, placement, align >> PAGE_SHIFT, false, acc_size, sg, robj, destroy)


