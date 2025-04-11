@@
expression pool, flags;
expression E1, E2;
expression ptr;
type T;
@@
(
-ptr = dma_pool_alloc(pool, flags, E1);
+ptr = dma_pool_zalloc(pool, flags, E1);
 ...
-memset(ptr, 0, sizeof(T));
)
|
(
-ptr = dma_pool_alloc(pool, flags, E1);
+ptr = dma_pool_zalloc(pool, flags, E1);
 ...
 if (!ptr)
 ...
-memset(ptr, 0, E2);
)
|
(
-ptr = dma_pool_alloc(pool, flags, E1);
+ptr = dma_pool_zalloc(pool, flags, E1);
 ...
-memset(ptr, 0, E2);
)