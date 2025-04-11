void rmnet_vnd_setup(struct net_device *rmnet_dev)
{
	rmnet_dev->netdev_ops = &rmnet_vnd_ops;
	rmnet_dev->mtu = RMNET_DFLT_PACKET_SIZE;
	rmnet_dev->needed_headroom = RMNET_NEEDED_HEADROOM;
	random_ether_addr(rmnet_dev->dev_addr);
	rmnet_dev->tx_queue_len = RMNET_TX_QUEUE_LEN;
	/* Raw IP mode */
	rmnet_dev->header_ops = NULL;  /* No header */
	rmnet_dev->type = ARPHRD_RAWIP;
	rmnet_dev->hard_header_len = 0;
	rmnet_dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
	rmnet_dev->needs_free_netdev = true;
	rmnet_dev->ethtool_ops = &rmnet_ethtool_ops;
	/* This perm addr will be used as interface identifier by IPv6 */
	rmnet_dev->addr_assign_type = NET_ADDR_RANDOM;
	eth_random_addr(rmnet_dev->perm_addr);
}