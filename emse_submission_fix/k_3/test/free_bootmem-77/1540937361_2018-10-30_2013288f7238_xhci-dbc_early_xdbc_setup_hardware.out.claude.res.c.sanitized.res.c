int __init early_xdbc_setup_hardware(void)
{
  int ret;
  if (!xdbc.xdbc_reg)
    return -ENODEV;
  xdbc_bios_handoff();
  raw_spin_lock_init(&xdbc.lock);
  ret = xdbc_early_setup();
  if (ret)
    {
      pr_notice("failed to setup the connection to host\n");
      xdbc_free_ring(&xdbc.evt_ring);
      xdbc_free_ring(&xdbc.out_ring);
      xdbc_free_ring(&xdbc.in_ring);
      if (xdbc.table_dma)
        memblock_free(xdbc.table_dma, PAGE_SIZE);
      if (xdbc.out_dma)
        memblock_free(xdbc.out_dma, PAGE_SIZE);
      xdbc.table_base = NULL;
      xdbc.out_buf = NULL;
    }
  return ret;
}