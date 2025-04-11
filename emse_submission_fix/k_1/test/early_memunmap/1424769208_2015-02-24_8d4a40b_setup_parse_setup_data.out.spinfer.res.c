static void __init parse_setup_data(void)
{
  struct setup_data *data;
  u64 pa_data, pa_next;
  pa_data = boot_params.hdr.setup_data;
  while (pa_data)
    {
      u32 data_len, data_type;
      data = early_memremap(pa_data, sizeof(*data));
      data_len = data->len + sizeof(struct setup_data);
      data_type = data->type;
      pa_next = data->next;
      early_iounmap(data, sizeof(*data));
      switch (data_type)
        {
        case SETUP_E820_EXT:
          parse_e820_ext(pa_data, data_len);
          break;
        case SETUP_DTB:
          add_dtb(pa_data);
          break;
        case SETUP_EFI:
          parse_efi_setup(pa_data, data_len);
          break;
        case SETUP_KASLR:
          parse_kaslr_setup(pa_data, data_len);
          break;
        default:
          break;
        }
      pa_data = pa_next;
    }
}