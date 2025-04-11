static int qla84xx_updatefw(struct bsg_job *bsg_job)
{
  struct fc_bsg_request *bsg_request = bsg_job->request;
  struct fc_bsg_reply *bsg_reply = bsg_job->reply;
  struct Scsi_Host *host = fc_bsg_to_shost(bsg_job);
  scsi_qla_host_t *vha = shost_priv(host);
  struct qla_hw_data *ha = vha->hw;
  struct verify_chip_entry_84xx *mn = NULL;
  dma_addr_t mn_dma, fw_dma;
  void *fw_buf = NULL;
  int rval = 0;
  uint32_t sg_cnt;
  uint32_t data_len;
  uint16_t options;
  uint32_t flag;
  uint32_t fw_ver;
  if (!IS_QLA84XX(ha))
    {
      ql_dbg(ql_dbg_user, vha, 0x7032, "Not 84xx, exiting.\n");
      return -EINVAL;
    }
  sg_cnt = dma_map_sg(&ha->pdev->dev, bsg_job->request_payload.sg_list, bsg_job->request_payload.sg_cnt, DMA_TO_DEVICE);
  if (!sg_cnt)
    {
      ql_log(ql_log_warn, vha, 0x7033, "dma_map_sg returned %d for request.\n", sg_cnt);
      return -ENOMEM;
    }
  if (sg_cnt != bsg_job->request_payload.sg_cnt)
    {
      ql_log(ql_log_warn,
             vha,
             0x7034,
             "DMA mapping resulted in different sg counts, "
             "request_sg_cnt: %x dma_request_sg_cnt: %x.\n",
             bsg_job->request_payload.sg_cnt,
             sg_cnt);
      rval = -EAGAIN;
      goto done_unmap_sg;
    }
  data_len = bsg_job->request_payload.payload_len;
  fw_buf = dma_alloc_coherent(&ha->pdev->dev, data_len, &fw_dma, GFP_KERNEL);
  if (!fw_buf)
    {
      ql_log(ql_log_warn, vha, 0x7035, "DMA alloc failed for fw_buf.\n");
      rval = -ENOMEM;
      goto done_unmap_sg;
    }
  sg_copy_to_buffer(bsg_job->request_payload.sg_list, bsg_job->request_payload.sg_cnt, fw_buf, data_len);
  mn = dma_pool_alloc(ha->s_dma_pool, GFP_KERNEL, &mn_dma);
  if (!mn)
    {
      ql_log(ql_log_warn, vha, 0x7036, "DMA alloc failed for fw buffer.\n");
      rval = -ENOMEM;
      goto done_free_fw_buf;
    }
  flag = bsg_request->rqst_data.h_vendor.vendor_cmd[1];
  fw_ver = le32_to_cpu(*((uint32_t *)((uint32_t *)fw_buf + 2)));
  memset(mn, 0, sizeof(struct access_chip_84xx));
  mn->entry_type = VERIFY_CHIP_IOCB_TYPE;
  mn->entry_count = 1;
  options = VCO_FORCE_UPDATE | VCO_END_OF_DATA;
  if (flag == A84_ISSUE_UPDATE_DIAGFW_CMD)
    options |= VCO_DIAG_FW;
  mn->options = cpu_to_le16(options);
  mn->fw_ver = cpu_to_le32(fw_ver);
  mn->fw_size = cpu_to_le32(data_len);
  mn->fw_seq_size = cpu_to_le32(data_len);
  mn->dseg_address[0] = cpu_to_le32(LSD(fw_dma));
  mn->dseg_address[1] = cpu_to_le32(MSD(fw_dma));
  mn->dseg_length = cpu_to_le32(data_len);
  mn->data_seg_cnt = cpu_to_le16(1);
  rval = qla2x00_issue_iocb_timeout(vha, mn, mn_dma, 0, 120);
  if (rval)
    {
      ql_log(ql_log_warn, vha, 0x7037, "Vendor request 84xx updatefw failed.\n");
      rval = (DID_ERROR << 16);
    }
  else
    {
      ql_dbg(ql_dbg_user, vha, 0x7038, "Vendor request 84xx updatefw completed.\n");
      bsg_job->reply_len = sizeof(struct fc_bsg_reply);
      bsg_reply->result = DID_OK;
    }
  dma_pool_free(ha->s_dma_pool, mn, mn_dma);
done_free_fw_buf:
  dma_free_coherent(&ha->pdev->dev, data_len, fw_buf, fw_dma);
done_unmap_sg:
  dma_unmap_sg(&ha->pdev->dev, bsg_job->request_payload.sg_list, bsg_job->request_payload.sg_cnt, DMA_TO_DEVICE);
  if (!rval)
    bsg_job_done(bsg_job, bsg_reply->result, bsg_reply->reply_payload_rcv_len);
  return rval;
}