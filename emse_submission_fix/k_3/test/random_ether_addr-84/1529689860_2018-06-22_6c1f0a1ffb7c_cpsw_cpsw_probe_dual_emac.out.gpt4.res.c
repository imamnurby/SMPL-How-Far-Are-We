static int cpsw_probe_dual_emac(struct cpsw_priv *priv)
{
  struct cpsw_common *cpsw = priv->cpsw;
  struct cpsw_platform_data *data = &cpsw->data;
  struct net_device *ndev;
  struct cpsw_priv *priv_sl2;
  int ret = 0;
  ndev = alloc_etherdev_mq(sizeof(struct cpsw_priv), CPSW_MAX_QUEUES);
  if (!ndev)
    {
      dev_err(cpsw->dev, "cpsw: error allocating net_device\n");
      return -ENOMEM;
    }
  priv_sl2 = netdev_priv(ndev);
  priv_sl2->cpsw = cpsw;
  priv_sl2->ndev = ndev;
  priv_sl2->dev = &ndev->dev;
  priv_sl2->msg_enable = netif_msg_init(debug_level, CPSW_DEBUG);
  if (is_valid_ether_addr(data->slave_data[1].mac_addr))
    {
      memcpy(priv_sl2->mac_addr, data->slave_data[1].mac_addr, ETH_ALEN);
      dev_info(cpsw->dev, "cpsw: Detected MACID = %pM\n", priv_sl2->mac_addr);
    }
  else
    {
      eth_random_addr(priv_sl2->mac_addr);
      dev_info(cpsw->dev, "cpsw: Random MACID = %pM\n", priv_sl2->mac_addr);
    }
  memcpy(ndev->dev_addr, priv_sl2->mac_addr, ETH_ALEN);
  priv_sl2->emac_port = 1;
  cpsw->slaves[1].ndev = ndev;
  ndev->features |= NETIF_F_HW_VLAN_CTAG_FILTER;
  ndev->netdev_ops = &cpsw_netdev_ops;
  ndev->ethtool_ops = &cpsw_ethtool_ops;

  SET_NETDEV_DEV(ndev, cpsw->dev);
  ret = register_netdev(ndev);
  if (ret)
    {
      dev_err(cpsw->dev, "cpsw: error registering net device\n");
      free_netdev(ndev);
      ret = -ENODEV;
    }
  return ret;
}