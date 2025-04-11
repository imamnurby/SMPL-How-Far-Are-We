@@
expression pool, flags, dma;
identifier ptr;
type T;
@@
- ptr = dma_pool_alloc(pool, flags, dma);
+ ptr = dma_pool_zalloc(pool, flags, dma);
  if (!ptr)
    { ... return ...; }
- memset(ptr, 0, sizeof(T));