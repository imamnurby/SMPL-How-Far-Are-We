static int dsi_bind(struct device *dev, struct device *master, void *data)
{
  struct platform_device *dsidev = to_platform_device(dev);
  const struct soc_device_attribute *soc;
  const struct dsi_module_id_data *d;
  u32 rev;
  int r, i;
  struct dsi_data *dsi;
  struct resource *dsi_mem;
  struct resource *res;
  dsi = devm_kzalloc(&dsidev->dev, sizeof(*dsi), GFP_KERNEL);
  if (!dsi)
    return -ENOMEM;
  dsi->pdev = dsidev;
  dev_set_drvdata(&dsidev->dev, dsi);
  spin_lock_init(&dsi->irq_lock);
  spin_lock_init(&dsi->errors_lock);
  dsi->errors = 0;
#ifdef CONFIG_OMAP2_DSS_COLLECT_IRQ_STATS
  spin_lock_init(&dsi->irq_stats_lock);
  dsi->irq_stats.last_reset = jiffies;
#endif
  mutex_init(&dsi->lock);
  sema_init(&dsi->bus_lock, 1);
  INIT_DEFERRABLE_WORK(&dsi->framedone_timeout_work, dsi_framedone_timeout_work_callback);
#ifdef DSI_CATCH_MISSING_TE
  setup_timer(&dsi->te_timer, dsi_te_timeout, 0UL);
  dsi->te_timer.data = 0;
#endif
  dsi_mem = platform_get_resource_byname(dsidev, IORESOURCE_MEM, "proto");
  dsi->proto_base = devm_ioremap_resource(&dsidev->dev, dsi_mem);
  if (IS_ERR(dsi->proto_base))
    return PTR_ERR(dsi->proto_base);
  res = platform_get_resource_byname(dsidev, IORESOURCE_MEM, "phy");
  dsi->phy_base = devm_ioremap_resource(&dsidev->dev, res);
  if (IS_ERR(dsi->phy_base))
    return PTR_ERR(dsi->phy_base);
  res = platform_get_resource_byname(dsidev, IORESOURCE_MEM, "pll");
  dsi->pll_base = devm_ioremap_resource(&dsidev->dev, res);
  if (IS_ERR(dsi->pll_base))
    return PTR_ERR(dsi->pll_base);
  dsi->irq = platform_get_irq(dsi->pdev, 0);
  if (dsi->irq < 0)
    {
      DSSERR("platform_get_irq failed\n");
      return -ENODEV;
    }
  r = devm_request_irq(&dsidev->dev, dsi->irq, omap_dsi_irq_handler, IRQF_SHARED, dev_name(&dsidev->dev), dsi->pdev);
  if (r < 0)
    {
      DSSERR("request_irq failed\n");
      return r;
    }
  soc = soc_device_match(dsi_soc_devices);
  if (soc)
    dsi->data = soc->data;
  else
    dsi->data = of_match_node(dsi_of_match, dev->of_node)->data;
  d = dsi->data->modules;
  while (d->address != 0 && d->address != dsi_mem->start)
    d++;
  if (d->address == 0)
    {
      DSSERR("unsupported DSI module\n");
      return -ENODEV;
    }
  dsi->module_id = d->id;
  if (dsi->data->model == DSI_MODEL_OMAP4 || dsi->data->model == DSI_MODEL_OMAP5)
    {
      struct device_node *np;

      np = of_find_node_by_name(NULL, dsi->data->model == DSI_MODEL_OMAP4 ? "omap4_padconf_global" : "omap5_padconf_global");
      if (!np)
        return -ENODEV;
      dsi->syscon = syscon_node_to_regmap(np);
      of_node_put(np);
    }

  for (i = 0; i < ARRAY_SIZE(dsi->vc); i++)
    {
      dsi->vc[i].source = DSI_VC_SOURCE_L4;
      dsi->vc[i].dssdev = NULL;
      dsi->vc[i].vc_id = 0;
    }
  r = dsi_get_clocks(dsidev);
  if (r)
    return r;
  dsi_init_pll_data(dsidev);
  pm_runtime_enable(&dsidev->dev);
  r = dsi_runtime_get(dsidev);
  if (r)
    goto err_runtime_get;
  rev = dsi_read_reg(dsidev, DSI_REVISION);
  dev_dbg(&dsidev->dev, "OMAP DSI rev %d.%d\n", FLD_GET(rev, 7, 4), FLD_GET(rev, 3, 0));

  if (dsi->data->quirks & DSI_QUIRK_GNQ)

    dsi->num_lanes_supported = 1 + REG_GET(dsidev, DSI_GNQ, 11, 9);
  else
    dsi->num_lanes_supported = 3;
  dsi->line_buffer_size = dsi_get_line_buf_size(dsidev);
  dsi_init_output(dsidev);
  r = dsi_probe_of(dsidev);
  if (r)
    {
      DSSERR("Invalid DSI DT data\n");
      goto err_probe_of;
    }
  r = of_platform_populate(dsidev->dev.of_node, NULL, NULL, &dsidev->dev);
  if (r)
    DSSERR("Failed to populate DSI child devices: %d\n", r);
  dsi_runtime_put(dsidev);
  if (dsi->module_id == 0)
    dss_debugfs_create_file("dsi1_regs", dsi1_dump_regs);
  else if (dsi->module_id == 1)
    dss_debugfs_create_file("dsi2_regs", dsi2_dump_regs);
#ifdef CONFIG_OMAP2_DSS_COLLECT_IRQ_STATS
  if (dsi->module_id == 0)
    dss_debugfs_create_file("dsi1_irqs", dsi1_dump_irqs);
  else if (dsi->module_id == 1)
    dss_debugfs_create_file("dsi2_irqs", dsi2_dump_irqs);
#endif
  return 0;
err_probe_of:
  dsi_uninit_output(dsidev);
  dsi_runtime_put(dsidev);
err_runtime_get:
  pm_runtime_disable(&dsidev->dev);
  return r;
}