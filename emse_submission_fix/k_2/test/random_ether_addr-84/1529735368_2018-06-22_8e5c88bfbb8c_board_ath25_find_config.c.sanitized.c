int __init ath25_find_config(phys_addr_t base, unsigned long size)
{
  const void __iomem *flash_base, *flash_limit;
  struct ath25_boarddata *config;
  unsigned int rcfg_size;
  int broken_boarddata = 0;
  const void __iomem *bcfg, *rcfg;
  u8 *board_data;
  u8 *radio_data;
  u8 *mac_addr;
  u32 offset;
  flash_base = ioremap_nocache(base, size);
  flash_limit = flash_base + size;
  ath25_board.config = NULL;
  ath25_board.radio = NULL;

  bcfg = find_board_config(flash_limit, false);

  if (!bcfg)
    {
      bcfg = find_board_config(flash_limit, true);
      broken_boarddata = 1;
    }
  if (!bcfg)
    {
      pr_warn("WARNING: No board configuration data found!\n");
      goto error;
    }
  board_data = kzalloc(BOARD_CONFIG_BUFSZ, GFP_KERNEL);
  if (!board_data)
    goto error;
  ath25_board.config = (struct ath25_boarddata *)board_data;
  memcpy_fromio(board_data, bcfg, 0x100);
  if (broken_boarddata)
    {
      pr_warn("WARNING: broken board data detected\n");
      config = ath25_board.config;
      if (is_zero_ether_addr(config->enet0_mac))
        {
          pr_info("Fixing up empty mac addresses\n");
          config->reset_config_gpio = 0xffff;
          config->sys_led_gpio = 0xffff;
          random_ether_addr(config->wlan0_mac);
          config->wlan0_mac[0] &= ~0x06;
          random_ether_addr(config->enet0_mac);
          random_ether_addr(config->enet1_mac);
        }
    }

  rcfg = find_radio_config(flash_limit, bcfg);
  if (!rcfg)
    {
      pr_warn("WARNING: Could not find Radio Configuration data\n");
      goto error;
    }
  radio_data = board_data + 0x100 + ((rcfg - bcfg) & 0xfff);
  ath25_board.radio = radio_data;
  offset = radio_data - board_data;
  pr_info("Radio config found at offset 0x%x (0x%x)\n", rcfg - bcfg, offset);
  rcfg_size = BOARD_CONFIG_BUFSZ - offset;
  memcpy_fromio(radio_data, rcfg, rcfg_size);
  mac_addr = &radio_data[0x1d * 2];
  if (is_broadcast_ether_addr(mac_addr))
    {
      pr_info("Radio MAC is blank; using board-data\n");
      ether_addr_copy(mac_addr, ath25_board.config->wlan0_mac);
    }
  iounmap(flash_base);
  return 0;
error:
  iounmap(flash_base);
  return -ENODEV;
}