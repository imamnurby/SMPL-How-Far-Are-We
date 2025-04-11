static void ieee80211_start_ibss_wq(struct work_struct *work)
{
  struct delayed_work *dwork = to_delayed_work(work);
  struct ieee80211_device *ieee = container_of(dwork, struct ieee80211_device, start_ibss_wq);

  if (!ieee->proto_started)
    {
      printk("==========oh driver down return\n");
      return;
    }
  mutex_lock(&ieee->wx_mutex);
  if (ieee->current_network.ssid_len == 0)
    {
      strcpy(ieee->current_network.ssid, IEEE80211_DEFAULT_TX_ESSID);
      ieee->current_network.ssid_len = strlen(IEEE80211_DEFAULT_TX_ESSID);
      ieee->ssid_set = 1;
    }

  ieee80211_softmac_check_all_nets(ieee);

  if (ieee->state == IEEE80211_NOLINK)
    ieee->current_network.channel = 6;

  if (ieee->state == IEEE80211_NOLINK)
    ieee80211_start_scan_syncro(ieee);

  if (ieee->state == IEEE80211_NOLINK)
    {
      printk("creating new IBSS cell\n");
      if (!ieee->wap_set)
        eth_random_addr(ieee->current_network.bssid);
      if (ieee->modulation & IEEE80211_CCK_MODULATION)
        {
          ieee->current_network.rates_len = 4;
          ieee->current_network.rates[0] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_1MB;
          ieee->current_network.rates[1] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_2MB;
          ieee->current_network.rates[2] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_5MB;
          ieee->current_network.rates[3] = IEEE80211_BASIC_RATE_MASK | IEEE80211_CCK_RATE_11MB;
        }
      else
        ieee->current_network.rates_len = 0;
      if (ieee->modulation & IEEE80211_OFDM_MODULATION)
        {
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
        }
      else
        {
          ieee->current_network.rates_ex_len = 0;
          ieee->rate = 22;
        }

      ieee->current_network.QoS_Enable = 0;
      ieee->SetWirelessMode(ieee->dev, IEEE_G);
      ieee->current_network.atim_window = 0;
      ieee->current_network.capability = WLAN_CAPABILITY_IBSS;
      if (ieee->short_slot)
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