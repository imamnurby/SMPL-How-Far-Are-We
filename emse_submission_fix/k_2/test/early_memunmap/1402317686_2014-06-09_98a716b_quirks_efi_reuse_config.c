int __init efi_reuse_config(u64 tables, int nr_tables) {
  int i, sz, ret = 0;
  void *p, *tablep;
  struct efi_setup_data *data;
  if (!efi_setup) return 0;
  if (!efi_enabled(EFI_64BIT)) return 0;
  data = early_memremap(efi_setup, sizeof(*data));
  if (!data) {
    ret = -ENOMEM;
    goto out;
  }
  if (!data->smbios) goto out_memremap;
  sz = sizeof(efi_config_table_64_t);
  p = tablep = early_memremap(tables, nr_tables * sz);
  if (!p) {
    pr_err("Could not map Configuration table!\n");
    ret = -ENOMEM;
    goto out_memremap;
  }
  for (i = 0; i < efi.systab->nr_tables; i++) {
    efi_guid_t guid;
    guid = ((efi_config_table_64_t *)p)->guid;
    if (!efi_guidcmp(guid, SMBIOS_TABLE_GUID)) ((efi_config_table_64_t *)p)->table = data->smbios;
    p += sz;
  }
  early_iounmap(tablep, nr_tables * sz);
out_memremap:
  early_iounmap(data, sizeof(*data));
out:
  return ret;
}