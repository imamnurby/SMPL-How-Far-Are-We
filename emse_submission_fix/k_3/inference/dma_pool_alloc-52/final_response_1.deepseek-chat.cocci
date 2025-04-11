@@
expression pool, flags, handle;
identifier mem;
type T;
@@
- mem = dma_pool_alloc(pool, flags, handle);
+ mem = dma_pool_zalloc(pool, flags, handle);
  if (mem == NULL)
    ... when != mem
    return ...;
- memset(mem, 0, sizeof(T));