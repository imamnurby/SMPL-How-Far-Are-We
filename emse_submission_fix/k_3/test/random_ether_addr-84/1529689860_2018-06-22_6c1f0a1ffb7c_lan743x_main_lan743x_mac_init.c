static int lan743x_mac_init(struct lan743x_adapter *adapter)
{
	bool mac_address_valid = true;
	struct net_device *netdev;
	u32 mac_addr_hi = 0;
	u32 mac_addr_lo = 0;
	u32 data;
	int ret;
	netdev = adapter->netdev;
	lan743x_csr_write(adapter, MAC_CR, MAC_CR_RST_);
	ret = lan743x_csr_wait_for_bit(adapter, MAC_CR, MAC_CR_RST_,
				       0, 1000, 20000, 100);
	if (ret)
		return ret;
	/* setup auto duplex, and speed detection */
	data = lan743x_csr_read(adapter, MAC_CR);
	data |= MAC_CR_ADD_ | MAC_CR_ASD_;
	data |= MAC_CR_CNTR_RST_;
	lan743x_csr_write(adapter, MAC_CR, data);
	mac_addr_hi = lan743x_csr_read(adapter, MAC_RX_ADDRH);
	mac_addr_lo = lan743x_csr_read(adapter, MAC_RX_ADDRL);
	adapter->mac_address[0] = mac_addr_lo & 0xFF;
	adapter->mac_address[1] = (mac_addr_lo >> 8) & 0xFF;
	adapter->mac_address[2] = (mac_addr_lo >> 16) & 0xFF;
	adapter->mac_address[3] = (mac_addr_lo >> 24) & 0xFF;
	adapter->mac_address[4] = mac_addr_hi & 0xFF;
	adapter->mac_address[5] = (mac_addr_hi >> 8) & 0xFF;
	if (((mac_addr_hi & 0x0000FFFF) == 0x0000FFFF) &&
	    mac_addr_lo == 0xFFFFFFFF) {
		mac_address_valid = false;
	} else if (!is_valid_ether_addr(adapter->mac_address)) {
		mac_address_valid = false;
	}
	if (!mac_address_valid)
		random_ether_addr(adapter->mac_address);
	lan743x_mac_set_address(adapter, adapter->mac_address);
	ether_addr_copy(netdev->dev_addr, adapter->mac_address);
	return 0;
}