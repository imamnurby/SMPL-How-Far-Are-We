int qla4xxx_set_chap(struct scsi_qla_host *ha, char *username, char *password,
		     uint16_t idx, int bidi)
{
	int ret = 0;
	int rval = QLA_ERROR;
	uint32_t offset = 0;
	struct ql4_chap_table *chap_table;
	uint32_t chap_size = 0;
	dma_addr_t chap_dma;
	chap_table = dma_pool_alloc(ha->chap_dma_pool, GFP_KERNEL, &chap_dma);
	if (chap_table == NULL) {
		ret =  -ENOMEM;
		goto exit_set_chap;
	}
	memset(chap_table, 0, sizeof(struct ql4_chap_table));
	if (bidi)
		chap_table->flags |= BIT_6; /* peer */
	else
		chap_table->flags |= BIT_7; /* local */
	chap_table->secret_len = strlen(password);
	strncpy(chap_table->secret, password, MAX_CHAP_SECRET_LEN - 1);
	strncpy(chap_table->name, username, MAX_CHAP_NAME_LEN - 1);
	chap_table->cookie = __constant_cpu_to_le16(CHAP_VALID_COOKIE);
	if (is_qla40XX(ha)) {
		chap_size = MAX_CHAP_ENTRIES_40XX * sizeof(*chap_table);
		offset = FLASH_CHAP_OFFSET;
	} else { /* Single region contains CHAP info for both ports which is
		  * divided into half for each port.
		  */
		chap_size = ha->hw.flt_chap_size / 2;
		offset = FLASH_RAW_ACCESS_ADDR + (ha->hw.flt_region_chap << 2);
		if (ha->port_num == 1)
			offset += chap_size;
	}
	offset += (idx * sizeof(struct ql4_chap_table));
	rval = qla4xxx_set_flash(ha, chap_dma, offset,
				sizeof(struct ql4_chap_table),
				FLASH_OPT_RMW_COMMIT);
	if (rval == QLA_SUCCESS && ha->chap_list) {
		/* Update ha chap_list cache */
		memcpy((struct ql4_chap_table *)ha->chap_list + idx,
		       chap_table, sizeof(struct ql4_chap_table));
	}
	dma_pool_free(ha->chap_dma_pool, chap_table, chap_dma);
	if (rval != QLA_SUCCESS)
		ret =  -EINVAL;
exit_set_chap:
	return ret;
}