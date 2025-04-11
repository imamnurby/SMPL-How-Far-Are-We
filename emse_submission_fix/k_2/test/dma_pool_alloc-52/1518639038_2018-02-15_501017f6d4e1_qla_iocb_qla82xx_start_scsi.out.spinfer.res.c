int qla82xx_start_scsi(srb_t *sp)
{
  int nseg;
  unsigned long flags;
  struct scsi_cmnd *cmd;
  uint32_t *clr_ptr;
  uint32_t index;
  uint32_t handle;
  uint16_t cnt;
  uint16_t req_cnt;
  uint16_t tot_dsds;
  struct device_reg_82xx __iomem *reg;
  uint32_t dbval;
  uint32_t *fcp_dl;
  uint8_t additional_cdb_len;
  struct ct6_dsd *ctx;
  struct scsi_qla_host *vha = sp->vha;
  struct qla_hw_data *ha = vha->hw;
  struct req_que *req = NULL;
  struct rsp_que *rsp = NULL;

  reg = &ha->iobase->isp82;
  cmd = GET_CMD_SP(sp);
  req = vha->req;
  rsp = ha->rsp_q_map[0];

  tot_dsds = 0;
  dbval = 0x04 | (ha->portnum << 5);

  if (vha->marker_needed != 0)
    {
      if (qla2x00_marker(vha, req, rsp, 0, 0, MK_SYNC_ALL) != QLA_SUCCESS)
        {
          ql_log(ql_log_warn, vha, 0x300c, "qla2x00_marker failed for cmd=%p.\n", cmd);
          return QLA_FUNCTION_FAILED;
        }
      vha->marker_needed = 0;
    }

  spin_lock_irqsave(&ha->hardware_lock, flags);

  handle = req->current_outstanding_cmd;
  for (index = 1; index < req->num_outstanding_cmds; index++)
    {
      handle++;
      if (handle == req->num_outstanding_cmds)
        handle = 1;
      if (!req->outstanding_cmds[handle])
        break;
    }
  if (index == req->num_outstanding_cmds)
    goto queuing_error;

  if (scsi_sg_count(cmd))
    {
      nseg = dma_map_sg(&ha->pdev->dev, scsi_sglist(cmd), scsi_sg_count(cmd), cmd->sc_data_direction);
      if (unlikely(!nseg))
        goto queuing_error;
    }
  else
    nseg = 0;
  tot_dsds = nseg;
  if (tot_dsds > ql2xshiftctondsd)
    {
      struct cmd_type_6 *cmd_pkt;
      uint16_t more_dsd_lists = 0;
      struct dsd_dma *dsd_ptr;
      uint16_t i;
      more_dsd_lists = qla24xx_calc_dsd_lists(tot_dsds);
      if ((more_dsd_lists + ha->gbl_dsd_inuse) >= NUM_DSD_CHAIN)
        {
          ql_dbg(ql_dbg_io, vha, 0x300d, "Num of DSD list %d is than %d for cmd=%p.\n", more_dsd_lists + ha->gbl_dsd_inuse, NUM_DSD_CHAIN, cmd);
          goto queuing_error;
        }
      if (more_dsd_lists <= ha->gbl_dsd_avail)
        goto sufficient_dsds;
      else
        more_dsd_lists -= ha->gbl_dsd_avail;
      for (i = 0; i < more_dsd_lists; i++)
        {
          dsd_ptr = kzalloc(sizeof(struct dsd_dma), GFP_ATOMIC);
          if (!dsd_ptr)
            {
              ql_log(ql_log_fatal,
                     vha,
                     0x300e,
                     "Failed to allocate memory for dsd_dma "
                     "for cmd=%p.\n",
                     cmd);
              goto queuing_error;
            }
          dsd_ptr->dsd_addr = dma_pool_alloc(ha->dl_dma_pool, GFP_ATOMIC, &dsd_ptr->dsd_list_dma);
          if (!dsd_ptr->dsd_addr)
            {
              kfree(dsd_ptr);
              ql_log(ql_log_fatal,
                     vha,
                     0x300f,
                     "Failed to allocate memory for dsd_addr "
                     "for cmd=%p.\n",
                     cmd);
              goto queuing_error;
            }
          list_add_tail(&dsd_ptr->list, &ha->gbl_dsd_list);
          ha->gbl_dsd_avail++;
        }
    sufficient_dsds:
      req_cnt = 1;
      if (req->cnt < (req_cnt + 2))
        {
          cnt = (uint16_t)RD_REG_DWORD_RELAXED(&reg->req_q_out[0]);
          if (req->ring_index < cnt)
            req->cnt = cnt - req->ring_index;
          else
            req->cnt = req->length - (req->ring_index - cnt);
          if (req->cnt < (req_cnt + 2))
            goto queuing_error;
        }
      ctx = sp->u.scmd.ctx = mempool_alloc(ha->ctx_mempool, GFP_ATOMIC);
      if (!ctx)
        {
          ql_log(ql_log_fatal, vha, 0x3010, "Failed to allocate ctx for cmd=%p.\n", cmd);
          goto queuing_error;
        }
      memset(ctx, 0, sizeof(struct ct6_dsd));
      ctx->fcp_cmnd = dma_pool_zalloc(ha->fcp_cmnd_dma_pool, GFP_ATOMIC,
                                      &ctx->fcp_cmnd_dma);
      if (!ctx->fcp_cmnd)
        {
          ql_log(ql_log_fatal, vha, 0x3011, "Failed to allocate fcp_cmnd for cmd=%p.\n", cmd);
          goto queuing_error;
        }

      INIT_LIST_HEAD(&ctx->dsd_list);
      ctx->dsd_use_cnt = 0;
      if (cmd->cmd_len > 16)
        {
          additional_cdb_len = cmd->cmd_len - 16;
          if ((cmd->cmd_len % 4) != 0)
            {

              ql_log(ql_log_warn,
                     vha,
                     0x3012,
                     "scsi cmd len %d not multiple of 4 "
                     "for cmd=%p.\n",
                     cmd->cmd_len,
                     cmd);
              goto queuing_error_fcp_cmnd;
            }
          ctx->fcp_cmnd_len = 12 + cmd->cmd_len + 4;
        }
      else
        {
          additional_cdb_len = 0;
          ctx->fcp_cmnd_len = 12 + 16 + 4;
        }
      cmd_pkt = (struct cmd_type_6 *)req->ring_ptr;
      cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

      clr_ptr = (uint32_t *)cmd_pkt + 2;
      memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
      cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

      cmd_pkt->nport_handle = cpu_to_le16(sp->fcport->loop_id);
      cmd_pkt->port_id[0] = sp->fcport->d_id.b.al_pa;
      cmd_pkt->port_id[1] = sp->fcport->d_id.b.area;
      cmd_pkt->port_id[2] = sp->fcport->d_id.b.domain;
      cmd_pkt->vp_index = sp->vha->vp_idx;

      if (qla24xx_build_scsi_type_6_iocbs(sp, cmd_pkt, tot_dsds))
        goto queuing_error_fcp_cmnd;
      int_to_scsilun(cmd->device->lun, &cmd_pkt->lun);
      host_to_fcp_swap((uint8_t *)&cmd_pkt->lun, sizeof(cmd_pkt->lun));

      int_to_scsilun(cmd->device->lun, &ctx->fcp_cmnd->lun);
      ctx->fcp_cmnd->additional_cdb_len = additional_cdb_len;
      if (cmd->sc_data_direction == DMA_TO_DEVICE)
        ctx->fcp_cmnd->additional_cdb_len |= 1;
      else if (cmd->sc_data_direction == DMA_FROM_DEVICE)
        ctx->fcp_cmnd->additional_cdb_len |= 2;

      if (ha->flags.fcp_prio_enabled)
        ctx->fcp_cmnd->task_attribute |= sp->fcport->fcp_prio << 3;
      memcpy(ctx->fcp_cmnd->cdb, cmd->cmnd, cmd->cmd_len);
      fcp_dl = (uint32_t *)(ctx->fcp_cmnd->cdb + 16 + additional_cdb_len);
      *fcp_dl = htonl((uint32_t)scsi_bufflen(cmd));
      cmd_pkt->fcp_cmnd_dseg_len = cpu_to_le16(ctx->fcp_cmnd_len);
      cmd_pkt->fcp_cmnd_dseg_address[0] = cpu_to_le32(LSD(ctx->fcp_cmnd_dma));
      cmd_pkt->fcp_cmnd_dseg_address[1] = cpu_to_le32(MSD(ctx->fcp_cmnd_dma));
      sp->flags |= SRB_FCP_CMND_DMA_VALID;
      cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));

      cmd_pkt->entry_count = (uint8_t)req_cnt;

      cmd_pkt->entry_status = (uint8_t)rsp->id;
    }
  else
    {
      struct cmd_type_7 *cmd_pkt;
      req_cnt = qla24xx_calc_iocbs(vha, tot_dsds);
      if (req->cnt < (req_cnt + 2))
        {
          cnt = (uint16_t)RD_REG_DWORD_RELAXED(&reg->req_q_out[0]);
          if (req->ring_index < cnt)
            req->cnt = cnt - req->ring_index;
          else
            req->cnt = req->length - (req->ring_index - cnt);
        }
      if (req->cnt < (req_cnt + 2))
        goto queuing_error;
      cmd_pkt = (struct cmd_type_7 *)req->ring_ptr;
      cmd_pkt->handle = MAKE_HANDLE(req->id, handle);

      clr_ptr = (uint32_t *)cmd_pkt + 2;
      memset(clr_ptr, 0, REQUEST_ENTRY_SIZE - 8);
      cmd_pkt->dseg_count = cpu_to_le16(tot_dsds);

      cmd_pkt->nport_handle = cpu_to_le16(sp->fcport->loop_id);
      cmd_pkt->port_id[0] = sp->fcport->d_id.b.al_pa;
      cmd_pkt->port_id[1] = sp->fcport->d_id.b.area;
      cmd_pkt->port_id[2] = sp->fcport->d_id.b.domain;
      cmd_pkt->vp_index = sp->vha->vp_idx;
      int_to_scsilun(cmd->device->lun, &cmd_pkt->lun);
      host_to_fcp_swap((uint8_t *)&cmd_pkt->lun, sizeof(cmd_pkt->lun));

      if (ha->flags.fcp_prio_enabled)
        cmd_pkt->task |= sp->fcport->fcp_prio << 3;

      memcpy(cmd_pkt->fcp_cdb, cmd->cmnd, cmd->cmd_len);
      host_to_fcp_swap(cmd_pkt->fcp_cdb, sizeof(cmd_pkt->fcp_cdb));
      cmd_pkt->byte_count = cpu_to_le32((uint32_t)scsi_bufflen(cmd));

      qla24xx_build_scsi_iocbs(sp, cmd_pkt, tot_dsds, req);

      cmd_pkt->entry_count = (uint8_t)req_cnt;

      cmd_pkt->entry_status = (uint8_t)rsp->id;
    }

  req->current_outstanding_cmd = handle;
  req->outstanding_cmds[handle] = sp;
  sp->handle = handle;
  cmd->host_scribble = (unsigned char *)(unsigned long)handle;
  req->cnt -= req_cnt;
  wmb();

  req->ring_index++;
  if (req->ring_index == req->length)
    {
      req->ring_index = 0;
      req->ring_ptr = req->ring;
    }
  else
    req->ring_ptr++;
  sp->flags |= SRB_DMA_VALID;

  dbval = dbval | (req->id << 8) | (req->ring_index << 16);
  if (ql2xdbwr)
    qla82xx_wr_32(ha, (uintptr_t __force)ha->nxdb_wr_ptr, dbval);
  else
    {
      WRT_REG_DWORD(ha->nxdb_wr_ptr, dbval);
      wmb();
      while (RD_REG_DWORD(ha->nxdb_rd_ptr) != dbval)
        {
          WRT_REG_DWORD(ha->nxdb_wr_ptr, dbval);
          wmb();
        }
    }

  if (vha->flags.process_response_queue && rsp->ring_ptr->signature != RESPONSE_PROCESSED)
    qla24xx_process_response_queue(vha, rsp);
  spin_unlock_irqrestore(&ha->hardware_lock, flags);
  return QLA_SUCCESS;
queuing_error_fcp_cmnd:
  dma_pool_free(ha->fcp_cmnd_dma_pool, ctx->fcp_cmnd, ctx->fcp_cmnd_dma);
queuing_error:
  if (tot_dsds)
    scsi_dma_unmap(cmd);
  if (sp->u.scmd.ctx)
    {
      mempool_free(sp->u.scmd.ctx, ha->ctx_mempool);
      sp->u.scmd.ctx = NULL;
    }
  spin_unlock_irqrestore(&ha->hardware_lock, flags);
  return QLA_FUNCTION_FAILED;
}