@@
expression E1, E2, E3;
expression ptr;
expression size;
@@
-ptr = dma_pool_alloc(E1, E2, E3);
+ptr = dma_pool_zalloc(E1, E2, E3);
...
when != ptr = E1
-memset(ptr, 0, size);


