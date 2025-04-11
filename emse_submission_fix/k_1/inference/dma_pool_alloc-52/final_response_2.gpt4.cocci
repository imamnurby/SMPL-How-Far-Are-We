@@
expression Pool, Flags, DmaPtr;
expression MemPtr;
@@
- MemPtr = dma_pool_alloc(Pool, Flags, &DmaPtr);
+ MemPtr = dma_pool_zalloc(Pool, Flags, &DmaPtr);
  ... when != MemPtr
- memset(MemPtr, 0, ...);