static int ipr_alloc_cmd_blks(struct ipr_ioa_cfg *ioa_cfg)
{
	struct ipr_cmnd *ipr_cmd;
	struct ipr_ioarcb *ioarcb;
	dma_addr_t dma_addr;
	int i, entries_each_hrrq, hrrq_id = 0;
	ioa_cfg->ipr_cmd_pool = dma_pool_create(IPR_NAME, &ioa_cfg->pdev->dev,
						sizeof(struct ipr_cmnd), 512, 0);
	if (!ioa_cfg->ipr_cmd_pool)
		return -ENOMEM;
	ioa_cfg->ipr_cmnd_list = kcalloc(IPR_NUM_CMD_BLKS, sizeof(struct ipr_cmnd *), GFP_KERNEL);
	ioa_cfg->ipr_cmnd_list_dma = kcalloc(IPR_NUM_CMD_BLKS, sizeof(dma_addr_t), GFP_KERNEL);
	if (!ioa_cfg->ipr_cmnd_list || !ioa_cfg->ipr_cmnd_list_dma) {
		ipr_free_cmd_blks(ioa_cfg);
		return -ENOMEM;
	}
	for (i = 0; i < ioa_cfg->hrrq_num; i++) {
		if (ioa_cfg->hrrq_num > 1) {
			if (i == 0) {
				entries_each_hrrq = IPR_NUM_INTERNAL_CMD_BLKS;
				ioa_cfg->hrrq[i].min_cmd_id = 0;
				ioa_cfg->hrrq[i].max_cmd_id =
					(entries_each_hrrq - 1);
			} else {
				entries_each_hrrq =
					IPR_NUM_BASE_CMD_BLKS/
					(ioa_cfg->hrrq_num - 1);
				ioa_cfg->hrrq[i].min_cmd_id =
					IPR_NUM_INTERNAL_CMD_BLKS +
					(i - 1) * entries_each_hrrq;
				ioa_cfg->hrrq[i].max_cmd_id =
					(IPR_NUM_INTERNAL_CMD_BLKS +
					i * entries_each_hrrq - 1);
			}
		} else {
			entries_each_hrrq = IPR_NUM_CMD_BLKS;
			ioa_cfg->hrrq[i].min_cmd_id = 0;
			ioa_cfg->hrrq[i].max_cmd_id = (entries_each_hrrq - 1);
		}
		ioa_cfg->hrrq[i].size = entries_each_hrrq;
	}
	BUG_ON(ioa_cfg->hrrq_num == 0);
	i = IPR_NUM_CMD_BLKS -
		ioa_cfg->hrrq[ioa_cfg->hrrq_num - 1].max_cmd_id - 1;
	if (i > 0) {
		ioa_cfg->hrrq[ioa_cfg->hrrq_num - 1].size += i;
		ioa_cfg->hrrq[ioa_cfg->hrrq_num - 1].max_cmd_id += i;
	}
	for (i = 0; i < IPR_NUM_CMD_BLKS; i++) {
		ipr_cmd = dma_pool_alloc(ioa_cfg->ipr_cmd_pool, GFP_KERNEL, &dma_addr);
		if (!ipr_cmd) {
			ipr_free_cmd_blks(ioa_cfg);
			return -ENOMEM;
		}
		memset(ipr_cmd, 0, sizeof(*ipr_cmd));
		ioa_cfg->ipr_cmnd_list[i] = ipr_cmd;
		ioa_cfg->ipr_cmnd_list_dma[i] = dma_addr;
		ioarcb = &ipr_cmd->ioarcb;
		ipr_cmd->dma_addr = dma_addr;
		if (ioa_cfg->sis64)
			ioarcb->a.ioarcb_host_pci_addr64 = cpu_to_be64(dma_addr);
		else
			ioarcb->a.ioarcb_host_pci_addr = cpu_to_be32(dma_addr);
		ioarcb->host_response_handle = cpu_to_be32(i << 2);
		if (ioa_cfg->sis64) {
			ioarcb->u.sis64_addr_data.data_ioadl_addr =
				cpu_to_be64(dma_addr + offsetof(struct ipr_cmnd, i.ioadl64));
			ioarcb->u.sis64_addr_data.ioasa_host_pci_addr =
				cpu_to_be64(dma_addr + offsetof(struct ipr_cmnd, s.ioasa64));
		} else {
			ioarcb->write_ioadl_addr =
				cpu_to_be32(dma_addr + offsetof(struct ipr_cmnd, i.ioadl));
			ioarcb->read_ioadl_addr = ioarcb->write_ioadl_addr;
			ioarcb->ioasa_host_pci_addr =
				cpu_to_be32(dma_addr + offsetof(struct ipr_cmnd, s.ioasa));
		}
		ioarcb->ioasa_len = cpu_to_be16(sizeof(struct ipr_ioasa));
		ipr_cmd->cmd_index = i;
		ipr_cmd->ioa_cfg = ioa_cfg;
		ipr_cmd->sense_buffer_dma = dma_addr +
			offsetof(struct ipr_cmnd, sense_buffer);
		ipr_cmd->ioarcb.cmd_pkt.hrrq_id = hrrq_id;
		ipr_cmd->hrrq = &ioa_cfg->hrrq[hrrq_id];
		list_add_tail(&ipr_cmd->queue, &ipr_cmd->hrrq->hrrq_free_q);
		if (i >= ioa_cfg->hrrq[hrrq_id].max_cmd_id)
			hrrq_id++;
	}
	return 0;
}