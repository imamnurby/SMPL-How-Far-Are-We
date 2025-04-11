@@
type T1, T2;
expression size_exp, placement_exp, align_exp, acc_size_exp;
identifier bdev_id, bo_id, init_func;
@@
(
T1 acc_size_exp;
|
)
...
  acc_size_exp =
    ttm_bo_dma_acc_size(&bdev_id, size_exp, sizeof(*bo_id));
...
- init_func(&bdev_id, bo_id, size_exp, T2, placement_exp,
-          align_exp >> PAGE_SHIFT, ..., NULL, acc_size_exp,
-          ...);
+ init_func(&bdev_id, bo_id, size_exp, T2, placement_exp,
+          align_exp >> PAGE_SHIFT, ..., acc_size_exp,
+          ...);
@@
type T;
identifier bdev_id, bo_id, align_exp, placement_exp, acc_size_exp;
expression size_exp;
@@
... when != acc_size_exp =
    ttm_bo_dma_acc_size(&bdev_id, size_exp, sizeof(*bo_id));
if (
    align_exp >> PAGE_SHIFT, ..., NULL, ...
) {
  T acc_size_exp;
  acc_size_exp =
    ttm_bo_dma_acc_size(&bdev_id, size_exp, sizeof(*bo_id));
  ...
}