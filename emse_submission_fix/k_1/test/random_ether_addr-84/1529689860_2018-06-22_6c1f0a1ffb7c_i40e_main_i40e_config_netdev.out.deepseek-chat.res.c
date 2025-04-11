static int i40e_config_netdev(struct i40e_vsi *vsi)
{
  struct i40e_pf *pf = vsi->back;
  struct i40e_hw *hw = &pf->hw;
  struct i40e_netdev_priv *np;
  struct net_device *netdev;
  u8 broadcast[ETH_ALEN];
  u8 mac_addr[ETH_ALEN];
  int etherdev_size;
  netdev_features_t hw_enc_features;
  netdev_features_t hw_features;
  etherdev_size = sizeof(struct i40e_netdev_priv);
  netdev = alloc_etherdev_mq(etherdev_size, vsi->alloc_queue_pairs);
  if (!netdev)
    return -ENOMEM;
  vsi->netdev = netdev;
  np = netdev_priv(netdev);
  np->vsi = vsi;
  hw_enc_features = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_HIGHDMA | NETIF_F_SOFT_FEATURES | NETIF_F_TSO | NETIF_F_TSO_ECN | NETIF_F_TSO6 | NETIF_F_GSO_GRE | NETIF_F_GSO_GRE_CSUM | NETIF_F_GSO_PARTIAL | NETIF_F_GSO_UDP_TUNNEL | NETIF_F_GSO_UDP_TUNNEL_CSUM | NETIF_F_SCTP_CRC | NETIF_F_RXHASH | NETIF_F_RXCSUM | 0;
  if (!(pf->hw_features & I40E_HW_OUTER_UDP_CSUM_CAPABLE))
    netdev->gso_partial_features |= NETIF_F_GSO_UDP_TUNNEL_CSUM;
  netdev->gso_partial_features |= NETIF_F_GSO_GRE_CSUM;
  netdev->hw_enc_features |= hw_enc_features;

  netdev->vlan_features |= hw_enc_features | NETIF_F_TSO_MANGLEID;
  if (!(pf->flags & I40E_FLAG_MFP_ENABLED))
    netdev->hw_features |= NETIF_F_NTUPLE | NETIF_F_HW_TC;
  hw_features = hw_enc_features | NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX;
  netdev->hw_features |= hw_features;
  netdev->features |= hw_features | NETIF_F_HW_VLAN_CTAG_FILTER;
  netdev->hw_enc_features |= NETIF_F_TSO_MANGLEID;
  if (vsi->type == I40E_VSI_MAIN)
    {
      SET_NETDEV_DEV(netdev, &pf->pdev->dev);
      ether_addr_copy(mac_addr, hw->mac.perm_addr);

      i40e_rm_default_mac_filter(vsi, mac_addr);
      spin_lock_bh(&vsi->mac_filter_hash_lock);
      i40e_add_mac_filter(vsi, mac_addr);
      spin_unlock_bh(&vsi->mac_filter_hash_lock);
    }
  else
    {

      snprintf(netdev->name, IFNAMSIZ, "%.*sv%%d", IFNAMSIZ - 4, pf->vsi[pf->lan_vsi]->netdev->name);
      eth_random_addr(mac_addr);
      spin_lock_bh(&vsi->mac_filter_hash_lock);
      i40e_add_mac_filter(vsi, mac_addr);
      spin_unlock_bh(&vsi->mac_filter_hash_lock);
    }

  eth_broadcast_addr(broadcast);
  spin_lock_bh(&vsi->mac_filter_hash_lock);
  i40e_add_mac_filter(vsi, broadcast);
  spin_unlock_bh(&vsi->mac_filter_hash_lock);
  ether_addr_copy(netdev->dev_addr, mac_addr);
  ether_addr_copy(netdev->perm_addr, mac_addr);
  netdev->priv_flags |= IFF_UNICAST_FLT;
  netdev->priv_flags |= IFF_SUPP_NOFCS;

  i40e_vsi_config_netdev_tc(vsi, vsi->tc_config.enabled_tc);
  netdev->netdev_ops = &i40e_netdev_ops;
  netdev->watchdog_timeo = 5 * HZ;
  i40e_set_ethtool_ops(netdev);

  netdev->min_mtu = ETH_MIN_MTU;
  netdev->max_mtu = I40E_MAX_RXBUFFER - I40E_PACKET_HDR_PAD;
  return 0;
}