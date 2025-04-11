@@
identifier pool, f;
expression gfp, dma;
@@
- f = dma_pool_alloc(pool, gfp, &dma);
+ f = dma_pool_zalloc(pool, gfp, &dma);
... when != f
  (
-  memset(f, 0, ...);
  |
-  memset((void *)f, 0, ...);
  )
...