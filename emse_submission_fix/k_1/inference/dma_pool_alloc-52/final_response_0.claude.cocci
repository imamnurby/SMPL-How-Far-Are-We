@@
expression pool, flags, dma;
expression ptr;
expression size;
@@
- ptr = dma_pool_alloc(pool, flags, dma);
+ ptr = dma_pool_zalloc(pool, flags, dma);
  ...
- memset(ptr, 0, size);