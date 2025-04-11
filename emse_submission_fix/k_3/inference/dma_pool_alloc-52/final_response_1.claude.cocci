@@
expression pool, flags, dma_addr;
expression ptr;
expression size;
@@
- ptr = dma_pool_alloc(pool, flags, dma_addr);
+ ptr = dma_pool_zalloc(pool, flags, dma_addr);
  <...
- memset(ptr, 0, size);
  ...>