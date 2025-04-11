static int gemini_ethernet_port_probe(struct platform_device *pdev)
{
  char *port_names[2] = {"ethernet0", "ethernet1"};
  struct gemini_ethernet_port *port;
  struct device *dev = &pdev->dev;
  struct gemini_ethernet *geth;
  struct net_device *netdev;
  struct resource *gmacres;
  struct resource *dmares;
  struct device *parent;
  unsigned int id;
  int irq;
  int ret;
  parent = dev->parent;
  geth = dev_get_drvdata(parent);
  if (!strcmp(dev_name(dev), "60008000.ethernet-port"))
    id = 0;
  else if (!strcmp(dev_name(dev), "6000c000.ethernet-port"))
    id = 1;
  else
    return -ENODEV;
  dev_info(dev, "probe %s ID %d\n", dev_name(dev), id);
  netdev = alloc_etherdev_mq(sizeof(*port), TX_QUEUE_NUM);
  if (!netdev)
    {
      dev_err(dev, "Can't allocate ethernet device #%d\n", id);
      return -ENOMEM;
    }
  port = netdev_priv(netdev);
  SET_NETDEV_DEV(netdev, dev);
  port->netdev = netdev;
  port->id = id;
  port->geth = geth;
  port->dev = dev;

  dmares = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!dmares)
    {
      dev_err(dev, "no DMA resource\n");
      return -ENODEV;
    }
  port->dma_base = devm_ioremap_resource(dev, dmares);
  if (IS_ERR(port->dma_base))
    return PTR_ERR(port->dma_base);

  gmacres = platform_get_resource(pdev, IORESOURCE_MEM, 1);
  if (!gmacres)
    {
      dev_err(dev, "no GMAC resource\n");
      return -ENODEV;
    }
  port->gmac_base = devm_ioremap_resource(dev, gmacres);
  if (IS_ERR(port->gmac_base))
    return PTR_ERR(port->gmac_base);

  irq = platform_get_irq(pdev, 0);
  if (irq <= 0)
    {
      dev_err(dev, "no IRQ\n");
      return irq ? irq : -ENODEV;
    }
  port->irq = irq;

  port->pclk = devm_clk_get(dev, "PCLK");
  if (IS_ERR(port->pclk))
    {
      dev_err(dev, "no PCLK\n");
      return PTR_ERR(port->pclk);
    }
  ret = clk_prepare_enable(port->pclk);
  if (ret)
    return ret;

  gemini_port_save_mac_addr(port);

  port->reset = devm_reset_control_get_exclusive(dev, NULL);
  if (IS_ERR(port->reset))
    {
      dev_err(dev, "no reset\n");
      return PTR_ERR(port->reset);
    }
  reset_control_reset(port->reset);
  usleep_range(100, 500);

  if (!id)
    geth->port0 = port;
  else
    geth->port1 = port;
  platform_set_drvdata(pdev, port);

  netdev->dev_id = port->id;
  netdev->irq = irq;
  netdev->netdev_ops = &gmac_351x_ops;
  netdev->ethtool_ops = &gmac_351x_ethtool_ops;
  spin_lock_init(&port->config_lock);
  gmac_clear_hw_stats(netdev);
  netdev->hw_features = GMAC_OFFLOAD_FEATURES;
  netdev->features |= GMAC_OFFLOAD_FEATURES | NETIF_F_GRO;
  port->freeq_refill = 0;
  netif_napi_add(netdev, &port->napi, gmac_napi_poll, DEFAULT_NAPI_WEIGHT);
  if (is_valid_ether_addr((void *)port->mac_addr))
    {
      memcpy(netdev->dev_addr, port->mac_addr, ETH_ALEN);
    }
  else
    {
      dev_dbg(dev, "ethernet address 0x%08x%08x%08x invalid\n", port->mac_addr[0], port->mac_addr[1], port->mac_addr[2]);
      dev_info(dev, "using a random ethernet address\n");
      eth_random_addr(netdev->dev_addr);
    }
  gmac_write_mac_address(netdev);
  ret = devm_request_threaded_irq(port->dev, port->irq, gemini_port_irq, gemini_port_irq_thread, IRQF_SHARED, port_names[port->id], port);
  if (ret)
    return ret;
  ret = register_netdev(netdev);
  if (!ret)
    {
      netdev_info(netdev, "irq %d, DMA @ 0x%pap, GMAC @ 0x%pap\n", port->irq, &dmares->start, &gmacres->start);
      ret = gmac_setup_phy(netdev);
      if (ret)
        netdev_info(netdev, "PHY init failed, deferring to ifup time\n");
      return 0;
    }
  port->netdev = NULL;
  free_netdev(netdev);
  return ret;
}