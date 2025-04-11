@@
identifier pool_func, alloc_func = dma_pool_alloc, zalloc_func = dma_pool_zalloc;
expression pool, flags, dma_addr_var, target_mem;
constant size;
@@
- target_mem = alloc_func(pool, flags, &dma_addr_var);
+ target_mem = zalloc_func(pool, flags, &dma_addr_var);
  ... when != target_mem
- memset(target_mem, 0, size);