@@
expression E0, E1, E2, E3, E4, E5, E6, E7, E8;
@@
- E0 = ttm_bo_init(&E1->ttm.bdev, &E2->bo, E3, E4, &E2->placement,                  E5 >> PAGE_SHIFT, false, NULL, E6, E7, E8,                  nouveau_bo_del_ttm); 
+ E0 = ttm_bo_init(&E1->ttm.bdev, &E2->bo, E3, E4, &E2->placement,                  E5 >> PAGE_SHIFT, false, E6, E7, E8, nouveau_bo_del_ttm); 
// Infered from: (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_nouveau_bo_nouveau_bo_new.{c,res.c}: nouveau_bo_new), (./ttm_bo_init-60/1519311151_2018-02-22_724daa4fd65d_nouveau_bo_nouveau_bo_new.{c,res.c}: nouveau_bo_new)
// Recall: 1.00, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 1/1(100%)


// ---------------------------------------------
