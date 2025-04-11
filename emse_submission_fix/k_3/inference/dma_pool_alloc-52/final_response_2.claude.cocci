@@
expression pool, gfp, ptr;
expression var;
expression size;
@@
- var = dma_pool_alloc(pool, gfp, ptr);
+ var = dma_pool_zalloc(pool, gfp, ptr);
  ...
  if (var == NULL) {
    ...
  }
  ...
- memset(var, 0, size);
@@
expression pool, gfp, ptr;
expression var;
expression size;
@@
- var = dma_pool_alloc(pool, gfp, ptr);
+ var = dma_pool_zalloc(pool, gfp, ptr);
  ...
  if (!var) {
    ...
  }
  ...
- memset(var, 0, size);
@@
expression pool, gfp, ptr;
expression var, member;
expression size;
@@
- var->member = dma_pool_alloc(pool, gfp, ptr);
+ var->member = dma_pool_zalloc(pool, gfp, ptr);
  ...
  if (!var->member) {
    ...
  }
  ...
- memset(var->member, 0, size);