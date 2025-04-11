static struct sta_info * ap_add_sta(struct ap_data *ap, u8 *addr)
{
	struct sta_info *sta;
	sta = kzalloc(sizeof(struct sta_info), GFP_ATOMIC);
	if (sta == NULL) {
		PDEBUG(DEBUG_AP, "AP: kmalloc failed\n");
		return NULL;
	}
	/* initialize STA info data */
	sta->local = ap->local;
	skb_queue_head_init(&sta->tx_buf);
	memcpy(sta->addr, addr, ETH_ALEN);
	atomic_inc(&sta->users);
	spin_lock_bh(&ap->sta_table_lock);
	list_add(&sta->list, &ap->sta_list);
	ap->num_sta++;
	ap_sta_hash_add(ap, sta);
	spin_unlock_bh(&ap->sta_table_lock);
	if (ap->proc) {
		struct add_sta_proc_data *entry;
		/* schedule a non-interrupt context process to add a procfs
		 * entry for the STA since procfs code use GFP_KERNEL */
		entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
		if (entry) {
			memcpy(entry->addr, sta->addr, ETH_ALEN);
			entry->next = ap->add_sta_proc_entries;
			ap->add_sta_proc_entries = entry;
			schedule_work(&ap->add_sta_proc_queue);
		} else
			printk(KERN_DEBUG "Failed to add STA proc data\n");
	}
#ifndef PRISM2_NO_KERNEL_IEEE80211_MGMT
	setup_timer(&sta->timer, ap_handle_timer, (unsigned long)sta);
	sta->timer.expires = jiffies + ap->max_inactivity;
	if (!ap->local->hostapd)
		add_timer(&sta->timer);
#endif /* PRISM2_NO_KERNEL_IEEE80211_MGMT */
	return sta;
}