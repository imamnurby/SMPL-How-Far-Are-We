static int netcp_create_interface(struct netcp_device *netcp_device,
				  struct device_node *node_interface)
{
	struct device *dev = netcp_device->device;
	struct device_node *node = dev->of_node;
	struct netcp_intf *netcp;
	struct net_device *ndev;
	resource_size_t size;
	struct resource res;
	void __iomem *efuse = NULL;
	u32 efuse_mac = 0;
	const void *mac_addr;
	u8 efuse_mac_addr[6];
	u32 temp[2];
	int ret = 0;
	ndev = alloc_etherdev_mqs(sizeof(*netcp), 1, 1);
	if (!ndev) {
		dev_err(dev, "Error allocating netdev\n");
		return -ENOMEM;
	}
	ndev->features |= NETIF_F_SG;
	ndev->features |= NETIF_F_HW_VLAN_CTAG_FILTER;
	ndev->hw_features = ndev->features;
	ndev->vlan_features |=  NETIF_F_SG;
	/* MTU range: 68 - 9486 */
	ndev->min_mtu = ETH_MIN_MTU;
	ndev->max_mtu = NETCP_MAX_FRAME_SIZE - (ETH_HLEN + ETH_FCS_LEN);
	netcp = netdev_priv(ndev);
	spin_lock_init(&netcp->lock);
	INIT_LIST_HEAD(&netcp->module_head);
	INIT_LIST_HEAD(&netcp->txhook_list_head);
	INIT_LIST_HEAD(&netcp->rxhook_list_head);
	INIT_LIST_HEAD(&netcp->addr_list);
	u64_stats_init(&netcp->stats.syncp_rx);
	u64_stats_init(&netcp->stats.syncp_tx);
	netcp->netcp_device = netcp_device;
	netcp->dev = netcp_device->device;
	netcp->ndev = ndev;
	netcp->ndev_dev  = &ndev->dev;
	netcp->msg_enable = netif_msg_init(netcp_debug_level, NETCP_DEBUG);
	netcp->tx_pause_threshold = MAX_SKB_FRAGS;
	netcp->tx_resume_threshold = netcp->tx_pause_threshold;
	netcp->node_interface = node_interface;
	ret = of_property_read_u32(node_interface, "efuse-mac", &efuse_mac);
	if (efuse_mac) {
		if (of_address_to_resource(node, NETCP_EFUSE_REG_INDEX, &res)) {
			dev_err(dev, "could not find efuse-mac reg resource\n");
			ret = -ENODEV;
			goto quit;
		}
		size = resource_size(&res);
		if (!devm_request_mem_region(dev, res.start, size,
					     dev_name(dev))) {
			dev_err(dev, "could not reserve resource\n");
			ret = -ENOMEM;
			goto quit;
		}
		efuse = devm_ioremap_nocache(dev, res.start, size);
		if (!efuse) {
			dev_err(dev, "could not map resource\n");
			devm_release_mem_region(dev, res.start, size);
			ret = -ENOMEM;
			goto quit;
		}
		emac_arch_get_mac_addr(efuse_mac_addr, efuse, efuse_mac);
		if (is_valid_ether_addr(efuse_mac_addr))
			ether_addr_copy(ndev->dev_addr, efuse_mac_addr);
		else
			eth_random_addr(ndev->dev_addr);
		devm_iounmap(dev, efuse);
		devm_release_mem_region(dev, res.start, size);
	} else {
		mac_addr = of_get_mac_address(node_interface);
		if (mac_addr)
			ether_addr_copy(ndev->dev_addr, mac_addr);
		else
			eth_random_addr(ndev->dev_addr);
	}
	ret = of_property_read_string(node_interface, "rx-channel",
				      &netcp->dma_chan_name);
	if (ret < 0) {
		dev_err(dev, "missing \"rx-channel\" parameter\n");
		ret = -ENODEV;
		goto quit;
	}
	ret = of_property_read_u32(node_interface, "rx-queue",
				   &netcp->rx_queue_id);
	if (ret < 0) {
		dev_warn(dev, "missing \"rx-queue\" parameter\n");
		netcp->rx_queue_id = KNAV_QUEUE_QPEND;
	}
	ret = of_property_read_u32_array(node_interface, "rx-queue-depth",
					 netcp->rx_queue_depths,
					 KNAV_DMA_FDQ_PER_CHAN);
	if (ret < 0) {
		dev_err(dev, "missing \"rx-queue-depth\" parameter\n");
		netcp->rx_queue_depths[0] = 128;
	}
	ret = of_property_read_u32_array(node_interface, "rx-pool", temp, 2);
	if (ret < 0) {
		dev_err(dev, "missing \"rx-pool\" parameter\n");
		ret = -ENODEV;
		goto quit;
	}
	netcp->rx_pool_size = temp[0];
	netcp->rx_pool_region_id = temp[1];
	ret = of_property_read_u32_array(node_interface, "tx-pool", temp, 2);
	if (ret < 0) {
		dev_err(dev, "missing \"tx-pool\" parameter\n");
		ret = -ENODEV;
		goto quit;
	}
	netcp->tx_pool_size = temp[0];
	netcp->tx_pool_region_id = temp[1];
	if (netcp->tx_pool_size < MAX_SKB_FRAGS) {
		dev_err(dev, "tx-pool size too small, must be atleast(%ld)\n",
			MAX_SKB_FRAGS);
		ret = -ENODEV;
		goto quit;
	}
	ret = of_property_read_u32(node_interface, "tx-completion-queue",
				   &netcp->tx_compl_qid);
	if (ret < 0) {
		dev_warn(dev, "missing \"tx-completion-queue\" parameter\n");
		netcp->tx_compl_qid = KNAV_QUEUE_QPEND;
	}
	/* NAPI register */
	netif_napi_add(ndev, &netcp->rx_napi, netcp_rx_poll, NETCP_NAPI_WEIGHT);
	netif_tx_napi_add(ndev, &netcp->tx_napi, netcp_tx_poll, NETCP_NAPI_WEIGHT);
	/* Register the network device */
	ndev->dev_id		= 0;
	ndev->watchdog_timeo	= NETCP_TX_TIMEOUT;
	ndev->netdev_ops	= &netcp_netdev_ops;
	SET_NETDEV_DEV(ndev, dev);
	list_add_tail(&netcp->interface_list, &netcp_device->interface_head);
	return 0;
quit:
	free_netdev(ndev);
	return ret;
}