static int flexrm_startup(struct mbox_chan *chan)
{
  u64 d;
  u32 val, off;
  int ret = 0;
  dma_addr_t next_addr;
  struct flexrm_ring *ring = chan->con_priv;

  ring->bd_base = dma_pool_alloc(ring->mbox->bd_pool, GFP_KERNEL, &ring->bd_dma_base);
  if (!ring->bd_base)
    {
      dev_err(ring->mbox->dev, "can't allocate BD memory for ring%d\n", ring->num);
      ret = -ENOMEM;
      goto fail;
    }

  for (off = 0; off < RING_BD_SIZE; off += RING_DESC_SIZE)
    {
      next_addr = off + RING_DESC_SIZE;
      if (next_addr == RING_BD_SIZE)
        next_addr = 0;
      next_addr += ring->bd_dma_base;
      if (RING_BD_ALIGN_CHECK(next_addr))
        d = flexrm_next_table_desc(RING_BD_TOGGLE_VALID(off), next_addr);
      else
        d = flexrm_null_desc(RING_BD_TOGGLE_INVALID(off));
      flexrm_write_desc(ring->bd_base + off, d);
    }

  ring->cmpl_base = dma_pool_zalloc(ring->mbox->cmpl_pool, GFP_KERNEL,
                                    &ring->cmpl_dma_base);
  if (!ring->cmpl_base)
    {
      dev_err(ring->mbox->dev, "can't allocate completion memory for ring%d\n", ring->num);
      ret = -ENOMEM;
      goto fail_free_bd_memory;
    }

  if (ring->irq == UINT_MAX)
    {
      dev_err(ring->mbox->dev, "ring%d IRQ not available\n", ring->num);
      ret = -ENODEV;
      goto fail_free_cmpl_memory;
    }
  ret = request_threaded_irq(ring->irq, flexrm_irq_event, flexrm_irq_thread, 0, dev_name(ring->mbox->dev), ring);
  if (ret)
    {
      dev_err(ring->mbox->dev, "failed to request ring%d IRQ\n", ring->num);
      goto fail_free_cmpl_memory;
    }
  ring->irq_requested = true;

  ring->irq_aff_hint = CPU_MASK_NONE;
  val = ring->mbox->num_rings;
  val = (num_online_cpus() < val) ? val / num_online_cpus() : 1;
  cpumask_set_cpu((ring->num / val) % num_online_cpus(), &ring->irq_aff_hint);
  ret = irq_set_affinity_hint(ring->irq, &ring->irq_aff_hint);
  if (ret)
    {
      dev_err(ring->mbox->dev, "failed to set IRQ affinity hint for ring%d\n", ring->num);
      goto fail_free_irq;
    }

  writel_relaxed(0x0, ring->regs + RING_CONTROL);

  val = BD_START_ADDR_VALUE(ring->bd_dma_base);
  writel_relaxed(val, ring->regs + RING_BD_START_ADDR);

  ring->bd_write_offset = readl_relaxed(ring->regs + RING_BD_WRITE_PTR);
  ring->bd_write_offset *= RING_DESC_SIZE;

  val = CMPL_START_ADDR_VALUE(ring->cmpl_dma_base);
  writel_relaxed(val, ring->regs + RING_CMPL_START_ADDR);

  ring->cmpl_read_offset = readl_relaxed(ring->regs + RING_CMPL_WRITE_PTR);
  ring->cmpl_read_offset *= RING_DESC_SIZE;

  readl_relaxed(ring->regs + RING_NUM_REQ_RECV_LS);
  readl_relaxed(ring->regs + RING_NUM_REQ_RECV_MS);
  readl_relaxed(ring->regs + RING_NUM_REQ_TRANS_LS);
  readl_relaxed(ring->regs + RING_NUM_REQ_TRANS_MS);
  readl_relaxed(ring->regs + RING_NUM_REQ_OUTSTAND);

  val = 0;
  val |= (ring->msi_timer_val << MSI_TIMER_VAL_SHIFT);
  val |= BIT(MSI_ENABLE_SHIFT);
  val |= (ring->msi_count_threshold & MSI_COUNT_MASK) << MSI_COUNT_SHIFT;
  writel_relaxed(val, ring->regs + RING_MSI_CONTROL);

  val = BIT(CONTROL_ACTIVE_SHIFT);
  writel_relaxed(val, ring->regs + RING_CONTROL);

  atomic_set(&ring->msg_send_count, 0);
  atomic_set(&ring->msg_cmpl_count, 0);
  return 0;
fail_free_irq:
  free_irq(ring->irq, ring);
  ring->irq_requested = false;
fail_free_cmpl_memory:
  dma_pool_free(ring->mbox->cmpl_pool, ring->cmpl_base, ring->cmpl_dma_base);
  ring->cmpl_base = NULL;
fail_free_bd_memory:
  dma_pool_free(ring->mbox->bd_pool, ring->bd_base, ring->bd_dma_base);
  ring->bd_base = NULL;
fail:
  return ret;
}