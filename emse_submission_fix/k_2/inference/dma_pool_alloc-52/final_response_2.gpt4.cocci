@@
identifier pool_function = "dma_pool_alloc", new_function = "dma_pool_zalloc";
expression pool, mem_flags, dma_address, allocated_mem;
type T;
@@
-   allocated_mem = pool_function(pool, mem_flags, &dma_address);
+   allocated_mem = new_function(pool, mem_flags, &dma_address);
@@
expression allocated_mem;
@@
-   memset(allocated_mem, 0, sizeof(T));