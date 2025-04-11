@@
expression alloc_function = dma_pool_alloc;
identifier pool;
expression flags;
identifier dma_variable;
identifier mem_variable;
type data_type;
@@
- mem_variable = alloc_function(pool, flags, &dma_variable);
+ mem_variable = dma_pool_zalloc(pool, flags, &dma_variable);
...
- memset(mem_variable, 0, sizeof(data_type));