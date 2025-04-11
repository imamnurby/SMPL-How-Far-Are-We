static int __init xdbc_init(void)
{
  unsigned long flags;
  void __iomem *base;
  int ret = 0;
  u32 offset;
  if (!(xdbc.flags & XDBC_FLAGS_INITIALIZED))
    return 0;

  if (early_xdbc_console.index == -1 || (early_xdbc_console.flags & CON_BOOT))
    {
      xdbc_trace("hardware not used anymore\n");
      goto free_and_quit;
    }
  base = ioremap_nocache(xdbc.xhci_start, xdbc.xhci_length);
  if (!base)
    {
      xdbc_trace("failed to remap the io address\n");
      ret = -ENOMEM;
      goto free_and_quit;
    }
  raw_spin_lock_irqsave(&xdbc.lock, flags);
  early_iounmap(xdbc.xhci_base, xdbc.xhci_length);
  xdbc.xhci_base = base;
  offset = xhci_find_next_ext_cap(xdbc.xhci_base, 0, XHCI_EXT_CAPS_DEBUG);
  xdbc.xdbc_reg = (struct xdbc_regs __iomem *)(xdbc.xhci_base + offset);
  raw_spin_unlock_irqrestore(&xdbc.lock, flags);
  kthread_run(xdbc_scrub_function, NULL, "%s", "xdbc");
  return 0;
free_and_quit:
  xdbc_free_ring(&xdbc.evt_ring);
  xdbc_free_ring(&xdbc.out_ring);
  xdbc_free_ring(&xdbc.in_ring);
  memblock_free(xdbc.table_dma, PAGE_SIZE);
  memblock_free(xdbc.out_dma, PAGE_SIZE);
  writel(0, &xdbc.xdbc_reg->control);
  early_iounmap(xdbc.xhci_base, xdbc.xhci_length);
  return ret;
}