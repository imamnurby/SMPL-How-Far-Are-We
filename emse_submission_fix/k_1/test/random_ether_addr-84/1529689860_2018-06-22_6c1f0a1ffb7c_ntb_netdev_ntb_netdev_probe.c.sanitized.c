static int ntb_netdev_probe(struct device *client_dev)
{
  struct ntb_dev *ntb;
  struct net_device *ndev;
  struct pci_dev *pdev;
  struct ntb_netdev *dev;
  int rc;
  ntb = dev_ntb(client_dev->parent);
  pdev = ntb->pdev;
  if (!pdev)
    return -ENODEV;
  ndev = alloc_etherdev(sizeof(*dev));
  if (!ndev)
    return -ENOMEM;
  SET_NETDEV_DEV(ndev, client_dev);
  dev = netdev_priv(ndev);
  dev->ndev = ndev;
  dev->pdev = pdev;
  ndev->features = NETIF_F_HIGHDMA;
  ndev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
  ndev->hw_features = ndev->features;
  ndev->watchdog_timeo = msecs_to_jiffies(NTB_TX_TIMEOUT_MS);
  random_ether_addr(ndev->perm_addr);
  memcpy(ndev->dev_addr, ndev->perm_addr, ndev->addr_len);
  ndev->netdev_ops = &ntb_netdev_ops;
  ndev->ethtool_ops = &ntb_ethtool_ops;
  ndev->min_mtu = 0;
  ndev->max_mtu = ETH_MAX_MTU;
  dev->qp = ntb_transport_create_queue(ndev, client_dev, &ntb_netdev_handlers);
  if (!dev->qp)
    {
      rc = -EIO;
      goto err;
    }
  ndev->mtu = ntb_transport_max_size(dev->qp) - ETH_HLEN;
  rc = register_netdev(ndev);
  if (rc)
    goto err1;
  list_add(&dev->list, &dev_list);
  dev_info(&pdev->dev, "%s created\n", ndev->name);
  return 0;
err1:
  ntb_transport_free_queue(dev->qp);
err:
  free_netdev(ndev);
  return rc;
}