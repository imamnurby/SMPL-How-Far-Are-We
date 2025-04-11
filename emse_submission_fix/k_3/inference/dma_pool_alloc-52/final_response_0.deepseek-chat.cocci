@@
expression pool, flags, dma;
identifier ptr;
@@
- ptr = dma_pool_alloc(pool, flags, dma);
+ ptr = dma_pool_zalloc(pool, flags, dma);
  if (ptr) {
-  memset(ptr, 0, ...);
  }