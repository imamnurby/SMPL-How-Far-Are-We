static struct net_device *
prism2_init_local_data(struct prism2_helper_functions *funcs, int card_idx,
		       struct device *sdev)
{
	struct net_device *dev;
	struct hostap_interface *iface;
	struct local_info *local;
	int len, i, ret;
	if (funcs == NULL)
		return NULL;
	len = strlen(dev_template);
	if (len >= IFNAMSIZ || strstr(dev_template, "%d") == NULL) {
		printk(KERN_WARNING "hostap: Invalid dev_template='%s'\n",
		       dev_template);
		return NULL;
	}
	len = sizeof(struct hostap_interface) +
		3 + sizeof(struct local_info) +
		3 + sizeof(struct ap_data);
	dev = alloc_etherdev(len);
	if (dev == NULL)
		return NULL;
	iface = netdev_priv(dev);
	local = (struct local_info *) ((((long) (iface + 1)) + 3) & ~3);
	local->ap = (struct ap_data *) ((((long) (local + 1)) + 3) & ~3);
	local->dev = iface->dev = dev;
	iface->local = local;
	iface->type = HOSTAP_INTERFACE_MASTER;
	INIT_LIST_HEAD(&local->hostap_interfaces);
	local->hw_module = THIS_MODULE;
#ifdef PRISM2_IO_DEBUG
	local->io_debug_enabled = 1;
#endif /* PRISM2_IO_DEBUG */
	local->func = funcs;
	local->func->cmd = hfa384x_cmd;
	local->func->read_regs = hfa384x_read_regs;
	local->func->get_rid = hfa384x_get_rid;
	local->func->set_rid = hfa384x_set_rid;
	local->func->hw_enable = prism2_hw_enable;
	local->func->hw_config = prism2_hw_config;
	local->func->hw_reset = prism2_hw_reset;
	local->func->hw_shutdown = prism2_hw_shutdown;
	local->func->reset_port = prism2_reset_port;
	local->func->schedule_reset = prism2_schedule_reset;
#ifdef PRISM2_DOWNLOAD_SUPPORT
	local->func->read_aux_fops = &prism2_download_aux_dump_proc_fops;
	local->func->download = prism2_download;
#endif /* PRISM2_DOWNLOAD_SUPPORT */
	local->func->tx = prism2_tx_80211;
	local->func->set_tim = prism2_set_tim;
	local->func->need_tx_headroom = 0; /* no need to add txdesc in
					    * skb->data (FIX: maybe for DMA bus
					    * mastering? */
	local->mtu = mtu;
	rwlock_init(&local->iface_lock);
	spin_lock_init(&local->txfidlock);
	spin_lock_init(&local->cmdlock);
	spin_lock_init(&local->baplock);
	spin_lock_init(&local->lock);
	spin_lock_init(&local->irq_init_lock);
	mutex_init(&local->rid_bap_mtx);
	if (card_idx < 0 || card_idx >= MAX_PARM_DEVICES)
		card_idx = 0;
	local->card_idx = card_idx;
	len = strlen(essid);
	memcpy(local->essid, essid,
	       len > MAX_SSID_LEN ? MAX_SSID_LEN : len);
	local->essid[MAX_SSID_LEN] = '\0';
	i = GET_INT_PARM(iw_mode, card_idx);
	if ((i >= IW_MODE_ADHOC && i <= IW_MODE_REPEAT) ||
	    i == IW_MODE_MONITOR) {
		local->iw_mode = i;
	} else {
		printk(KERN_WARNING "prism2: Unknown iw_mode %d; using "
		       "IW_MODE_MASTER\n", i);
		local->iw_mode = IW_MODE_MASTER;
	}
	local->channel = GET_INT_PARM(channel, card_idx);
	local->beacon_int = GET_INT_PARM(beacon_int, card_idx);
	local->dtim_period = GET_INT_PARM(dtim_period, card_idx);
	local->wds_max_connections = 16;
	local->tx_control = HFA384X_TX_CTRL_FLAGS;
	local->manual_retry_count = -1;
	local->rts_threshold = 2347;
	local->fragm_threshold = 2346;
	local->rssi_to_dBm = 100; /* default; to be overriden by
				   * cnfDbmAdjust, if available */
	local->auth_algs = PRISM2_AUTH_OPEN | PRISM2_AUTH_SHARED_KEY;
	local->sram_type = -1;
	local->scan_channel_mask = 0xffff;
	local->monitor_type = PRISM2_MONITOR_RADIOTAP;
	/* Initialize task queue structures */
	INIT_WORK(&local->reset_queue, handle_reset_queue);
	INIT_WORK(&local->set_multicast_list_queue,
		  hostap_set_multicast_list_queue);
	INIT_WORK(&local->set_tim_queue, handle_set_tim_queue);
	INIT_LIST_HEAD(&local->set_tim_list);
	spin_lock_init(&local->set_tim_lock);
	INIT_WORK(&local->comms_qual_update, handle_comms_qual_update);
	/* Initialize tasklets for handling hardware IRQ related operations
	 * outside hw IRQ handler */
#define HOSTAP_TASKLET_INIT(q, f, d) \
do { memset((q), 0, sizeof(*(q))); (q)->func = (f); (q)->data = (d); } \
while (0)
	HOSTAP_TASKLET_INIT(&local->bap_tasklet, hostap_bap_tasklet,
			    (unsigned long) local);
	HOSTAP_TASKLET_INIT(&local->info_tasklet, hostap_info_tasklet,
			    (unsigned long) local);
	hostap_info_init(local);
	HOSTAP_TASKLET_INIT(&local->rx_tasklet,
			    hostap_rx_tasklet, (unsigned long) local);
	skb_queue_head_init(&local->rx_list);
	HOSTAP_TASKLET_INIT(&local->sta_tx_exc_tasklet,
			    hostap_sta_tx_exc_tasklet, (unsigned long) local);
	skb_queue_head_init(&local->sta_tx_exc_list);
	INIT_LIST_HEAD(&local->cmd_queue);
	init_waitqueue_head(&local->hostscan_wq);
	lib80211_crypt_info_init(&local->crypt_info, dev->name, &local->lock);
	init_timer(&local->passive_scan_timer);
	local->passive_scan_timer.data = (unsigned long) local;
	local->passive_scan_timer.function = hostap_passive_scan;
	init_timer(&local->tick_timer);
	local->tick_timer.data = (unsigned long) local;
	local->tick_timer.function = hostap_tick_timer;
	local->tick_timer.expires = jiffies + 2 * HZ;
	add_timer(&local->tick_timer);
	INIT_LIST_HEAD(&local->bss_list);
	hostap_setup_dev(dev, local, HOSTAP_INTERFACE_MASTER);
	dev->type = ARPHRD_IEEE80211;
	dev->header_ops = &hostap_80211_ops;
	rtnl_lock();
	ret = dev_alloc_name(dev, "wifi%d");
	SET_NETDEV_DEV(dev, sdev);
	if (ret >= 0)
		ret = register_netdevice(dev);
	prism2_set_lockdep_class(dev);
	rtnl_unlock();
	if (ret < 0) {
		printk(KERN_WARNING "%s: register netdevice failed!\n",
		       dev_info);
		goto fail;
	}
	printk(KERN_INFO "%s: Registered netdevice %s\n", dev_info, dev->name);
	hostap_init_data(local);
	return dev;
 fail:
	free_netdev(dev);
	return NULL;
}