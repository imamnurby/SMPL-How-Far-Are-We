@@
identifier I8;
expression E0, E1, E2, E3, E4, E5, E6, E7;
@@
- E0 = ttm_bo_init(&E1->ttm.bdev, &E2->bo, E3, E4, &E2->placement,                  E5 >> PAGE_SHIFT, false, NULL, E6, E7, E7, I8); 
+ E0 = ttm_bo_init(&E1->ttm.bdev, &E2->bo, E3, ttm_bo_type_device,                  &E2->placement, E5 >> PAGE_SHIFT, false, E6, NULL, NULL,                  ast_bo_ttm_destroy); 
// Infered from: (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_ast_ttm_ast_bo_create.{c,res.c}: ast_bo_create), (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_ast_ttm_ast_bo_create.{c,res.c}: ast_bo_create)
// Recall: 0.33, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
@@
identifier I9;
expression E0, E1, E2, E3, E4, E5, E6, E7, E8;
@@
- E0 = ttm_bo_init(&E1->ttm.bdev, &E2->bo, E3, E4, &E2->placement,                  E5 >> PAGE_SHIFT, false, NULL, E6, E7, E8, I9); 
+ E0 = ttm_bo_init(&E1->ttm.bdev, &E2->bo, E3, E4, &E2->placement,                  E5 >> PAGE_SHIFT, false, E6, E7, E8, nouveau_bo_del_ttm); 
// Infered from: (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_nouveau_bo_nouveau_bo_new.{c,res.c}: nouveau_bo_new), (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_nouveau_bo_nouveau_bo_new.{c,res.c}: nouveau_bo_new)
// False positives: (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_ast_ttm_ast_bo_create.res.c: ast_bo_create)
// Recall: 0.50, Precision: 0.75, Matching recall: 0.75

// ---------------------------------------------
@@
expression E0, E1, E2, E3, E4, E5;
@@
- E0 = ttm_bo_init(&E1->mman.bdev, &E2->tbo, E3, E4, &E2->placement, 0, !E5,                  NULL, E3, NULL, NULL, &qxl_ttm_bo_destroy); 
+ E0 = ttm_bo_init(&E1->mman.bdev, &E2->tbo, E3, E4, &E2->placement, 0, !E5,                  E3, NULL, NULL, &qxl_ttm_bo_destroy); 
// Infered from: (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_qxl_object_qxl_bo_create.{c,res.c}: qxl_bo_create), (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_qxl_object_qxl_bo_create.{c,res.c}: qxl_bo_create)
// Recall: 0.33, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 3 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 3/3(100%)


// ---------------------------------------------
