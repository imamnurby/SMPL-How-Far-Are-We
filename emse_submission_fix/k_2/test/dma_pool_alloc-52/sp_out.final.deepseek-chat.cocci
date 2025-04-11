@@
expression e1, e2, e3;
identifier x;
@@
- x = dma_pool_alloc(e1, e2, e3);
+ x = dma_pool_zalloc(e1, e2, e3);
  ...
- memset(x, 0, ...);


