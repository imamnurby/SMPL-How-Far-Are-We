void __init parse_e820_ext(u64 phys_addr, u32 data_len) {
  int entries;
  struct e820entry *extmap;
  struct setup_data *sdata;
  sdata = early_memremap(phys_addr, data_len);
  entries = sdata->len / sizeof(struct e820entry);
  extmap = (struct e820entry *)(sdata->data);
  __append_e820_map(extmap, entries);
  sanitize_e820_map(e820.map, ARRAY_SIZE(e820.map), &e820.nr_map);
  early_memunmap(sdata, data_len);
  printk(KERN_INFO "e820: extended physical RAM map:\n");
  e820_print_map("extended");
}