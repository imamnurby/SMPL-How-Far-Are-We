struct dasd_block *dasd_alloc_block(void)
{
  struct dasd_block *block;
  block = kzalloc(sizeof(*block), GFP_ATOMIC);
  if (!block)
    return ERR_PTR(-ENOMEM);

  atomic_set(&block->open_count, -1);
  atomic_set(&block->tasklet_scheduled, 0);
  tasklet_init(&block->tasklet, (void (*)(unsigned long))dasd_block_tasklet, (unsigned long)block);
  INIT_LIST_HEAD(&block->ccw_queue);
  spin_lock_init(&block->queue_lock);
  setup_timer(&block->timer, dasd_block_timeout, (unsigned long)block);
  spin_lock_init(&block->profile.lock);
  return block;
}