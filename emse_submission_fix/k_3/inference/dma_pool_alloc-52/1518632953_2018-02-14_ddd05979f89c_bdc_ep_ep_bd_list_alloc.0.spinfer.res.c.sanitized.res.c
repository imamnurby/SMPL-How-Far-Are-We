static int ep_bd_list_alloc(struct bdc_ep *ep)
{
  struct bd_table *prev_table = NULL;
  int index, num_tabs, bd_p_tab;
  struct bdc *bdc = ep->bdc;
  struct bd_table *bd_table;
  dma_addr_t dma;
  if (usb_endpoint_xfer_isoc(ep->desc))
    num_tabs = NUM_TABLES_ISOCH;
  else
    num_tabs = NUM_TABLES;
  bd_p_tab = NUM_BDS_PER_TABLE;

  dev_dbg(bdc->dev, "%s ep:%p num_tabs:%d\n", __func__, ep, num_tabs);

  ep->bd_list.bd_table_array = kzalloc(num_tabs * sizeof(struct bd_table *), GFP_ATOMIC);
  if (!ep->bd_list.bd_table_array)
    return -ENOMEM;

  for (index = 0; index < num_tabs; index++)
    {

      bd_table = kzalloc(sizeof(struct bd_table), GFP_ATOMIC);
      if (!bd_table)
        goto fail;
      bd_table->start_bd = dma_pool_zalloc(bdc->bd_table_pool, GFP_ATOMIC, &dma);
      if (!bd_table->start_bd)
        {
          kfree(bd_table);
          goto fail;
        }
      bd_table->dma = dma;
      dev_dbg(bdc->dev, "index:%d start_bd:%p dma=%08llx prev_table:%p\n", index, bd_table->start_bd, (unsigned long long)bd_table->dma, prev_table);
      ep->bd_list.bd_table_array[index] = bd_table;
      if (prev_table)
        chain_table(prev_table, bd_table, bd_p_tab);
      prev_table = bd_table;
    }
  chain_table(prev_table, ep->bd_list.bd_table_array[0], bd_p_tab);

  ep->bd_list.num_tabs = num_tabs;
  ep->bd_list.max_bdi = (num_tabs * bd_p_tab) - 1;
  ep->bd_list.num_tabs = num_tabs;
  ep->bd_list.num_bds_table = bd_p_tab;
  ep->bd_list.eqp_bdi = 0;
  ep->bd_list.hwd_bdi = 0;
  return 0;
fail:

  ep_bd_list_free(ep, num_tabs);
  return -ENOMEM;
}