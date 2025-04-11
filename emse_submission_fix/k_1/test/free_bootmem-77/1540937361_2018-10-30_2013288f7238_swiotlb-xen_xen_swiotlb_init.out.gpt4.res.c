int __ref xen_swiotlb_init(int verbose, bool early)
{
  unsigned long bytes, order;
  int rc = -ENOMEM;
  enum xen_swiotlb_err m_ret = XEN_SWIOTLB_UNKNOWN;
  unsigned int repeat = 3;
  xen_io_tlb_nslabs = swiotlb_nr_tbl();
retry:
  bytes = xen_set_nslabs(xen_io_tlb_nslabs);
  order = get_order(xen_io_tlb_nslabs << IO_TLB_SHIFT);

  if (early)
    xen_io_tlb_start = memblock_alloc(PAGE_ALIGN(bytes), PAGE_SIZE);
  else
    {
#define SLABS_PER_PAGE (1 << (PAGE_SHIFT - IO_TLB_SHIFT))
#define IO_TLB_MIN_SLABS ((1 << 20) >> IO_TLB_SHIFT)
      while ((SLABS_PER_PAGE << order) > IO_TLB_MIN_SLABS)
        {
          xen_io_tlb_start = (void *)xen_get_swiotlb_free_pages(order);
          if (xen_io_tlb_start)
            break;
          order--;
        }
      if (order != get_order(bytes))
        {
          pr_warn("Warning: only able to allocate %ld MB for software IO TLB\n", (PAGE_SIZE << order) >> 20);
          xen_io_tlb_nslabs = SLABS_PER_PAGE << order;
          bytes = xen_io_tlb_nslabs << IO_TLB_SHIFT;
        }
    }
  if (!xen_io_tlb_start)
    {
      m_ret = XEN_SWIOTLB_ENOMEM;
      goto error;
    }
  xen_io_tlb_end = xen_io_tlb_start + bytes;

  rc = xen_swiotlb_fixup(xen_io_tlb_start, bytes, xen_io_tlb_nslabs);
  if (rc)
    {
      if (early)
        free_bootmem(__pa(xen_io_tlb_start), PAGE_ALIGN(bytes));
      else
        {
          free_pages((unsigned long)xen_io_tlb_start, order);
          xen_io_tlb_start = NULL;
        }
      m_ret = XEN_SWIOTLB_EFIXUP;
      goto error;
    }
  start_dma_addr = xen_virt_to_bus(xen_io_tlb_start);
  if (early)
    {
      if (swiotlb_init_with_tbl(xen_io_tlb_start, xen_io_tlb_nslabs, verbose))
        panic("Cannot allocate SWIOTLB buffer");
      rc = 0;
    }
  else
    rc = swiotlb_late_init_with_tbl(xen_io_tlb_start, xen_io_tlb_nslabs);
  if (!rc)
    swiotlb_set_max_segment(PAGE_SIZE);
  return rc;
error:
  if (repeat--)
    {
      xen_io_tlb_nslabs = max(1024UL, (xen_io_tlb_nslabs >> 1));
      pr_info("Lowering to %luMB\n", (xen_io_tlb_nslabs << IO_TLB_SHIFT) >> 20);
      goto retry;
    }
  pr_err("%s (rc:%d)\n", xen_swiotlb_error(m_ret), rc);
  if (early)
    panic("%s (rc:%d)", xen_swiotlb_error(m_ret), rc);
  else
    free_pages((unsigned long)xen_io_tlb_start, order);
  return rc;
}