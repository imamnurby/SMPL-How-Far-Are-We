static void ieee80211_start_ibss_wq(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct ieee80211_device *ieee = container_of(dwork, struct ieee80211_device, start_ibss_wq);
	/* iwconfig mode ad-hoc will schedule this and return
	 * on the other hand this will block further iwconfig SET
	 * operations because of the wx_mutex hold.
	 * Anyway some most set operations set a flag to speed-up
	 * (abort) this wq (when syncro scanning) before sleeping
	 * on the semaphore
	 */
	if (!ieee->proto_started) {
		printk("==========oh driver down return\n");
		return;
	}
	mutex_lock(&ieee->wx_mutex);
	if (ieee->current_network.ssid_len == 0) {
		strcpy(ieee->current_network.ssid, IEEE80211_DEFAULT_TX_ESSID);
		ieee->current_network.ssid_len = strlen(IEEE80211_DEFAULT_TX_ESSID);
		ieee->ssid_set = 1;
	}
	/* check if we have this cell in our network list */
	ieee80211_softmac_check_all_nets(ieee);
//	if((IS_DOT11D_ENABLE(ieee)) && (ieee->state == IEEE80211_NOLINK))
	if (ieee->state == IEEE80211_NOLINK)
		ieee->current_network.channel = 6;
	/* if not then the state is not linked. Maybe the user switched to
	 * ad-hoc mode just after being in monitor mode, or just after
	 * being very few time in managed mode (so the card have had no
	 * time to scan all the chans..) or we have just run up the iface
	 * after setting ad-hoc mode. So we have to give another try..
	 * Here, in ibss mode, should be safe to do this without extra care
	 * (in bss mode we had to make sure no-one tryed to associate when
	 * we had just checked the ieee->state and we was going to start the
	 * scan) beacause in ibss mode the ieee80211_new_net function, when
	 * finds a good net, just set the ieee->state to IEEE80211_LINKED,
	 * so, at worst, we waste a bit of time to initiate an unneeded syncro
	 * scan, that will stop at the first round because it sees the state
	 * associated.
	 */
	if (ieee->state == IEEE80211_NOLINK)
		ieee80211_start_scan_syncro(ieee);
	/* the network definitively is not here.. create a new cell */
	if (ieee->state == IEEE80211_NOLINK) {
		printk("creating new IBSS cell\n");
		if(!ieee->wap_set)
			eth_random_addr(ieee->current_network.bssid);
		if(ieee->modulation & IEEE80211_CCK_MODULATION){
			ieee->current_network.rates_len = 4;
			ieee->current_network.rates[0] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_1MB;
			ieee->current_network.rates[1] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_2MB;
			ieee->current_network.rates[2] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_5MB;
			ieee->current_network.rates[3] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_11MB;
		}else
			ieee->current_network.rates_len = 0;
		if(ieee->modulation & IEEE80211_OFDM_MODULATION){
			ieee->current_network.rates_ex_len = 8;
			ieee->current_network.rates_ex[0] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_6MB;
			ieee->current_network.rates_ex[1] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_9MB;
			ieee->current_network.rates_ex[2] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_12MB;
			ieee->current_network.rates_ex[3] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_18MB;
			ieee->current_network.rates_ex[4] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_24MB;
			ieee->current_network.rates_ex[5] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_36MB;
			ieee->current_network.rates_ex[6] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_48MB;
			ieee->current_network.rates_ex[7] = IEEE80211_BASIC_RATE_MASK | IEEE80211_OFDM_RATE_54MB;
			ieee->rate = 108;
		}else{
			ieee->current_network.rates_ex_len = 0;
			ieee->rate = 22;
		}
		// By default, WMM function will be disabled in IBSS mode
		ieee->current_network.QoS_Enable = 0;
		ieee->SetWirelessMode(ieee->dev, IEEE_G);
		ieee->current_network.atim_window = 0;
		ieee->current_network.capability = WLAN_CAPABILITY_IBSS;
		if(ieee->short_slot)
			ieee->current_network.capability |= WLAN_CAPABILITY_SHORT_SLOT;
	}
	ieee->state = IEEE80211_LINKED;
	ieee->set_chan(ieee->dev, ieee->current_network.channel);
	ieee->link_change(ieee->dev);
	notify_wx_assoc_event(ieee);
	ieee80211_start_send_beacons(ieee);
	if (ieee->data_hard_resume)
		ieee->data_hard_resume(ieee->dev);
	netif_carrier_on(ieee->dev);
	mutex_unlock(&ieee->wx_mutex);
}