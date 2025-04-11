@@
identifier pool_function;
identifier buffer_name, dma_addr;
identifier struct_type; // The relevant struct type being initialized by memset
@@
- buffer_name = dma_pool_alloc(pool_function, ..., &dma_addr);
+ buffer_name = dma_pool_zalloc(pool_function, ..., &dma_addr);
  ...
- memset(buffer_name, 0, sizeof(struct_type));


