int qla4xxx_get_chap(struct scsi_qla_host *ha, char *username, char *password, uint16_t idx)
{
  int ret = 0;
  int rval = QLA_ERROR;
  uint32_t offset = 0, chap_size;
  struct ql4_chap_table *chap_table;
  dma_addr_t chap_dma;
  chap_table = dma_pool_alloc(ha->chap_dma_pool, GFP_KERNEL, &chap_dma);
  if (chap_table == NULL)
    return -ENOMEM;
  chap_size = sizeof(struct ql4_chap_table);
  memset(chap_table, 0, chap_size);
  if (is_qla40XX(ha))
    offset = FLASH_CHAP_OFFSET | (idx * chap_size);
  else
    {
      offset = FLASH_RAW_ACCESS_ADDR + (ha->hw.flt_region_chap << 2);

      if (ha->port_num == 1)
        offset += (ha->hw.flt_chap_size / 2);
      offset += (idx * chap_size);
    }
  rval = qla4xxx_get_flash(ha, chap_dma, offset, chap_size);
  if (rval != QLA_SUCCESS)
    {
      ret = -EINVAL;
      goto exit_get_chap;
    }
  DEBUG2(ql4_printk(KERN_INFO, ha, "Chap Cookie: x%x\n", __le16_to_cpu(chap_table->cookie)));
  if (__le16_to_cpu(chap_table->cookie) != CHAP_VALID_COOKIE)
    {
      ql4_printk(KERN_ERR, ha, "No valid chap entry found\n");
      goto exit_get_chap;
    }
  strlcpy(password, chap_table->secret, QL4_CHAP_MAX_SECRET_LEN);
  strlcpy(username, chap_table->name, QL4_CHAP_MAX_NAME_LEN);
  chap_table->cookie = __constant_cpu_to_le16(CHAP_VALID_COOKIE);
exit_get_chap:
  dma_pool_free(ha->chap_dma_pool, chap_table, chap_dma);
  return ret;
}