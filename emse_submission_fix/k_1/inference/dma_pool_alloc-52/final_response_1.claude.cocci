@@
expression E1, E2, E3;
identifier ptr;
expression size;
@@
- ptr = dma_pool_alloc(E1, E2, E3);
+ ptr = dma_pool_zalloc(E1, E2, E3);
  ... when != ptr
(
- memset(ptr, 0, size);
|
)