@@
expression pool, flags, dma;
identifier mem;
@@
- mem = dma_pool_alloc(pool, flags, dma);
+ mem = dma_pool_zalloc(pool, flags, dma);
  ... when != mem
- memset(mem, 0, ...);