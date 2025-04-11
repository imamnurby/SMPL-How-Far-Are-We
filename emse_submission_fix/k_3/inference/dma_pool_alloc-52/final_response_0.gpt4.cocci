@@
identifier pool_func = dma_pool_alloc, pool_zalloc = dma_pool_zalloc;
expression pool, gfp, dma_ptr;
type T;
@@
- T *res = pool_func(pool, gfp, &dma_ptr);
+ T *res = pool_zalloc(pool, gfp, &dma_ptr);
  ... when != res == NULL
      when != (T *)res = pool_func(...)
- memset(res, 0, sizeof(T));