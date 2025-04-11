@@
expression E_bdev, E_bo, E_size, E_type, E_acc_size, E_last_arg;
identifier I_placement, I_align;
@@
- ttm_bo_init(E_bdev, E_bo, E_size, E_type, &I_placement, I_align, false, NULL, E_acc_size, E_last_arg)
+ ttm_bo_init(E_bdev, E_bo, E_size, E_type, &I_placement, I_align, false, E_acc_size, E_last_arg)


