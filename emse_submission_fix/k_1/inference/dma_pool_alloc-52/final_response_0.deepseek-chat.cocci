@@
expression pool, flags, dma;
identifier table;
expression size;
@@
- table->start_bd = dma_pool_alloc(pool, flags, dma);
+ table->start_bd = dma_pool_zalloc(pool, flags, dma);
  if (!table->start_bd)
    {
      ...
    }
- memset(table->start_bd, 0, size);