@@
identifier alloc_func = "dma_pool_alloc", zalloc_func = "dma_pool_zalloc";
identifier pool, ptr, dma;
expression size_expr;
@@
- ptr = alloc_func(pool, GFP_ATOMIC, &dma);
+ ptr = zalloc_func(pool, GFP_ATOMIC, &dma);
  ...
- memset(ptr, 0, size_expr);