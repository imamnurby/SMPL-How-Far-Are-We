static void lan78xx_init_mac_address(struct lan78xx_net *dev)
{
  u32 addr_lo, addr_hi;
  int ret;
  u8 addr[6];
  ret = lan78xx_read_reg(dev, RX_ADDRL, &addr_lo);
  ret = lan78xx_read_reg(dev, RX_ADDRH, &addr_hi);
  addr[0] = addr_lo & 0xFF;
  addr[1] = (addr_lo >> 8) & 0xFF;
  addr[2] = (addr_lo >> 16) & 0xFF;
  addr[3] = (addr_lo >> 24) & 0xFF;
  addr[4] = addr_hi & 0xFF;
  addr[5] = (addr_hi >> 8) & 0xFF;
  if (!is_valid_ether_addr(addr))
    {
      if (!eth_platform_get_mac_address(&dev->udev->dev, addr))
        {

          netif_dbg(dev, ifup, dev->net, "MAC address read from Device Tree");
        }
      else if (((lan78xx_read_eeprom(dev, EEPROM_MAC_OFFSET, ETH_ALEN, addr) == 0) || (lan78xx_read_otp(dev, EEPROM_MAC_OFFSET, ETH_ALEN, addr) == 0)) && is_valid_ether_addr(addr))
        {

          netif_dbg(dev, ifup, dev->net, "MAC address read from EEPROM");
        }
      else
        {

          random_ether_addr(addr);
          netif_dbg(dev, ifup, dev->net, "MAC address set to random addr");
        }
      addr_lo = addr[0] | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24);
      addr_hi = addr[4] | (addr[5] << 8);
      ret = lan78xx_write_reg(dev, RX_ADDRL, addr_lo);
      ret = lan78xx_write_reg(dev, RX_ADDRH, addr_hi);
    }
  ret = lan78xx_write_reg(dev, MAF_LO(0), addr_lo);
  ret = lan78xx_write_reg(dev, MAF_HI(0), addr_hi | MAF_HI_VALID_);
  ether_addr_copy(dev->net->dev_addr, addr);
}