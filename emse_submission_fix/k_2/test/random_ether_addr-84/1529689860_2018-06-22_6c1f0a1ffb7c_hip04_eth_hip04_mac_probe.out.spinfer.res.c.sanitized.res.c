static int hip04_mac_probe(struct platform_device *pdev)
{
  struct device *d = &pdev->dev;
  struct device_node *node = d->of_node;
  struct of_phandle_args arg;
  struct net_device *ndev;
  struct hip04_priv *priv;
  struct resource *res;
  int irq;
  int ret;
  ndev = alloc_etherdev(sizeof(struct hip04_priv));
  if (!ndev)
    return -ENOMEM;
  priv = netdev_priv(ndev);
  priv->ndev = ndev;
  platform_set_drvdata(pdev, ndev);
  SET_NETDEV_DEV(ndev, &pdev->dev);
  res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  priv->base = devm_ioremap_resource(d, res);
  if (IS_ERR(priv->base))
    {
      ret = PTR_ERR(priv->base);
      goto init_fail;
    }
  ret = of_parse_phandle_with_fixed_args(node, "port-handle", 2, 0, &arg);
  if (ret < 0)
    {
      dev_warn(d, "no port-handle\n");
      goto init_fail;
    }
  priv->port = arg.args[0];
  priv->chan = arg.args[1] * RX_DESC_NUM;
  hrtimer_init(&priv->tx_coalesce_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

  priv->tx_coalesce_frames = TX_DESC_NUM * 3 / 4;
  priv->tx_coalesce_usecs = 200;
  priv->tx_coalesce_timer.function = tx_done;
  priv->map = syscon_node_to_regmap(arg.np);
  if (IS_ERR(priv->map))
    {
      dev_warn(d, "no syscon hisilicon,hip04-ppe\n");
      ret = PTR_ERR(priv->map);
      goto init_fail;
    }
  priv->phy_mode = of_get_phy_mode(node);
  if (priv->phy_mode < 0)
    {
      dev_warn(d, "not find phy-mode\n");
      ret = -EINVAL;
      goto init_fail;
    }
  irq = platform_get_irq(pdev, 0);
  if (irq <= 0)
    {
      ret = -EINVAL;
      goto init_fail;
    }
  ret = devm_request_irq(d, irq, hip04_mac_interrupt, 0, pdev->name, ndev);
  if (ret)
    {
      netdev_err(ndev, "devm_request_irq failed\n");
      goto init_fail;
    }
  priv->phy_node = of_parse_phandle(node, "phy-handle", 0);
  if (priv->phy_node)
    {
      priv->phy = of_phy_connect(ndev, priv->phy_node, &hip04_adjust_link, 0, priv->phy_mode);
      if (!priv->phy)
        {
          ret = -EPROBE_DEFER;
          goto init_fail;
        }
    }
  INIT_WORK(&priv->tx_timeout_task, hip04_tx_timeout_task);
  ndev->netdev_ops = &hip04_netdev_ops;
  ndev->ethtool_ops = &hip04_ethtool_ops;
  ndev->watchdog_timeo = TX_TIMEOUT;
  ndev->priv_flags |= IFF_UNICAST_FLT;
  ndev->irq = irq;
  netif_napi_add(ndev, &priv->napi, hip04_rx_poll, NAPI_POLL_WEIGHT);
  hip04_reset_ppe(priv);
  if (priv->phy_mode == PHY_INTERFACE_MODE_MII)
    hip04_config_port(ndev, SPEED_100, DUPLEX_FULL);
  hip04_config_fifo(priv);
  eth_random_addr(ndev->dev_addr);
  hip04_update_mac_address(ndev);
  ret = hip04_alloc_ring(ndev, d);
  if (ret)
    {
      netdev_err(ndev, "alloc ring fail\n");
      goto alloc_fail;
    }
  ret = register_netdev(ndev);
  if (ret)
    {
      free_netdev(ndev);
      goto alloc_fail;
    }
  return 0;
alloc_fail:
  hip04_free_ring(ndev, d);
init_fail:
  of_node_put(priv->phy_node);
  free_netdev(ndev);
  return ret;
}