@@
identifier I2, I3;
expression E1, E0, E4, E5;
@@
- E0 = dma_pool_alloc(E1->I2, I3, &E4); 
+ E0 = dma_pool_zalloc(E1->I2, I3, &E4); 
  ...
- memset(E0, 0, E5); 
// Infered from: (./dma_pool_alloc-52/1518639038_2018-02-15_501017f6d4e1_qla_bsg_qla84xx_updatefw.{c.sanitized.c,res.c.sanitized.res.c}: qla84xx_updatefw), (./dma_pool_alloc-52/1518634820_2018-02-15_8d35a9dc4244_ql4_mbx_qla4xxx_set_chap.{c.sanitized.c,res.c.sanitized.res.c}: qla4xxx_set_chap), (./dma_pool_alloc-52/1518632953_2018-02-14_ddd05979f89c_bdc_ep_ep_bd_list_alloc.{c.sanitized.c,res.c.sanitized.res.c}: ep_bd_list_alloc)
// Recall: 1.00, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 3/3(100%)


// ---------------------------------------------
