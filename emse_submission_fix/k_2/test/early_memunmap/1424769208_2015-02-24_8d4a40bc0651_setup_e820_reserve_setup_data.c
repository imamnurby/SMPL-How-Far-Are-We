static void __init e820_reserve_setup_data(void) {
  struct setup_data *data;
  u64 pa_data;
  int found = 0;
  pa_data = boot_params.hdr.setup_data;
  while (pa_data) {
    data = early_memremap(pa_data, sizeof(*data));
    e820_update_range(pa_data, sizeof(*data) + data->len, E820_RAM, E820_RESERVED_KERN);
    found = 1;
    pa_data = data->next;
    early_iounmap(data, sizeof(*data));
  }
  if (!found) return;
  sanitize_e820_map(e820.map, ARRAY_SIZE(e820.map), &e820.nr_map);
  memcpy(&e820_saved, &e820, sizeof(struct e820map));
  printk(KERN_INFO "extended physical RAM map:\n");
  e820_print_map("reserve setup_data");
}