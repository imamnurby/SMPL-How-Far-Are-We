@@
expression E0, E1, E2, E3;
@@
- E0->start_bd = dma_pool_alloc(E1->bd_table_pool, GFP_ATOMIC, &E2); 
+ E0->start_bd = dma_pool_zalloc(E1->bd_table_pool, GFP_ATOMIC, &E2); 
  ...
- memset(E0->start_bd, 0, E3 * sizeof(struct bdc_bd )); 
// Infered from: (./dma_pool_alloc-52/1518632953_2018-02-14_ddd05979f89c_bdc_ep_ep_bd_list_alloc.{c.sanitized.c,res.c.sanitized.res.c}: ep_bd_list_alloc)
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
