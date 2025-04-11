@@
expression dev, bo, size, type, placement, page_align, interruptible;
expression acc_size, sg, res_obj, destroy;
expression ret;
@@
(
ret = ttm_bo_init(dev, bo, size, type, placement, page_align, interruptible,
-                 NULL, acc_size,
+                 acc_size,
                  sg, res_obj, destroy)
|
ret = ttm_bo_init(dev, bo, size, type, placement, page_align, interruptible,
-                 NULL, acc_size,
+                 acc_size,
                  sg, res_obj, destroy);
)


