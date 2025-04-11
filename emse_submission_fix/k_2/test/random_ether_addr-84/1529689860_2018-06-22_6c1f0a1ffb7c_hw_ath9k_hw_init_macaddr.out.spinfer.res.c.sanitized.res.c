static void ath9k_hw_init_macaddr(struct ath_hw *ah)
{
  struct ath_common *common = ath9k_hw_common(ah);
  int i;
  u16 eeval;
  static const u32 EEP_MAC[] = {EEP_MAC_LSW, EEP_MAC_MID, EEP_MAC_MSW};

  if (is_valid_ether_addr(common->macaddr))
    return;
  for (i = 0; i < 3; i++)
    {
      eeval = ah->eep_ops->get_eeprom(ah, EEP_MAC[i]);
      common->macaddr[2 * i] = eeval >> 8;
      common->macaddr[2 * i + 1] = eeval & 0xff;
    }
  if (is_valid_ether_addr(common->macaddr))
    return;
  ath_err(common, "eeprom contains invalid mac address: %pM\n", common->macaddr);
  eth_random_addr(common->macaddr);
  ath_err(common, "random mac address will be used: %pM\n", common->macaddr);
  return;
}