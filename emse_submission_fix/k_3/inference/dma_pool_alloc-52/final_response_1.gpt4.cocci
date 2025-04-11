@@
identifier pool;
identifier alloc_func = "dma_pool_zalloc";
expression dest, args;
type ttype;
@@
- dest = dma_pool_alloc(pool, args);
+ dest = alloc_func(pool, args);
... when != dest == NULL
    when any
- memset(dest, 0, sizeof(ttype));