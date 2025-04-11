@@
expression pool, flags, dma;
identifier ptr;
expression size;
@@
- ptr = dma_pool_alloc(pool, flags, dma);
+ ptr = dma_pool_zalloc(pool, flags, dma);
  if (!ptr)
    { ... return ...; }
- memset(ptr, 0, size);