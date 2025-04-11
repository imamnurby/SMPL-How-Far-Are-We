@@
expression pool, flags, handle;
identifier ptr;
type T;
@@
- ptr = dma_pool_alloc(pool, flags, handle);
+ ptr = dma_pool_zalloc(pool, flags, handle);
  ... when != ptr
- memset(ptr, 0, sizeof(T));


