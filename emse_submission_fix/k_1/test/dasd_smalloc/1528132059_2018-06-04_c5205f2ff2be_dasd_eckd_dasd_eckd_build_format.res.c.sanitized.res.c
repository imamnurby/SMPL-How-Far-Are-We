static struct dasd_ccw_req *dasd_eckd_build_format(struct dasd_device *base, struct format_data_t *fdata, int enable_pav)
{
  struct dasd_eckd_private *base_priv;
  struct dasd_eckd_private *start_priv;
  struct dasd_device *startdev = NULL;
  struct dasd_ccw_req *fcp;
  struct eckd_count *ect;
  struct ch_t address;
  struct ccw1 *ccw;
  void *data;
  int rpt;
  int cplength, datasize;
  int i, j;
  int intensity = 0;
  int r0_perm;
  int nr_tracks;
  int use_prefix;
  if (enable_pav)
    startdev = dasd_alias_get_start_dev(base);
  if (!startdev)
    startdev = base;
  start_priv = startdev->private;
  base_priv = base->private;
  rpt = recs_per_track(&base_priv->rdc_data, 0, fdata->blksize);
  nr_tracks = fdata->stop_unit - fdata->start_unit + 1;

  if (fdata->intensity & 0x10)
    {
      r0_perm = 0;
      intensity = fdata->intensity & ~0x10;
    }
  else
    {
      r0_perm = 1;
      intensity = fdata->intensity;
    }
  use_prefix = base_priv->features.feature[8] & 0x01;
  switch (intensity)
    {
    case 0x00:
    case 0x08:
      cplength = 2 + (rpt * nr_tracks);
      if (use_prefix)
        datasize = sizeof(struct PFX_eckd_data) + sizeof(struct LO_eckd_data) + rpt * nr_tracks * sizeof(struct eckd_count);
      else
        datasize = sizeof(struct DE_eckd_data) + sizeof(struct LO_eckd_data) + rpt * nr_tracks * sizeof(struct eckd_count);
      break;
    case 0x01:
    case 0x09:
      cplength = 2 + rpt * nr_tracks;
      if (use_prefix)
        datasize = sizeof(struct PFX_eckd_data) + sizeof(struct LO_eckd_data) + sizeof(struct eckd_count) + rpt * nr_tracks * sizeof(struct eckd_count);
      else
        datasize = sizeof(struct DE_eckd_data) + sizeof(struct LO_eckd_data) + sizeof(struct eckd_count) + rpt * nr_tracks * sizeof(struct eckd_count);
      break;
    case 0x04:
    case 0x0c:
      cplength = 3;
      if (use_prefix)
        datasize = sizeof(struct PFX_eckd_data) + sizeof(struct LO_eckd_data) + sizeof(struct eckd_count);
      else
        datasize = sizeof(struct DE_eckd_data) + sizeof(struct LO_eckd_data) + sizeof(struct eckd_count);
      break;
    default:
      dev_warn(&startdev->cdev->dev, "An I/O control call used incorrect flags 0x%x\n", fdata->intensity);
      return ERR_PTR(-EINVAL);
    }

  fcp = dasd_smalloc_request(DASD_ECKD_MAGIC, cplength, datasize, startdev, NULL);
  if (IS_ERR(fcp))
    return fcp;
  start_priv->count++;
  data = fcp->data;
  ccw = fcp->cpaddr;
  switch (intensity & ~0x08)
    {
    case 0x00:
      if (use_prefix)
        {
          prefix(ccw++, (struct PFX_eckd_data *)data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_WRITE_CKD, base, startdev);

          if (r0_perm)
            ((struct PFX_eckd_data *)data)->define_extent.ga_extended |= 0x04;
          data += sizeof(struct PFX_eckd_data);
        }
      else
        {
          define_extent(ccw++, (struct DE_eckd_data *)data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_WRITE_CKD, startdev, 0);

          if (r0_perm)
            ((struct DE_eckd_data *)data)->ga_extended |= 0x04;
          data += sizeof(struct DE_eckd_data);
        }
      ccw[-1].flags |= CCW_FLAG_CC;
      locate_record(ccw++, (struct LO_eckd_data *)data, fdata->start_unit, 0, rpt * nr_tracks, DASD_ECKD_CCW_WRITE_CKD, base, fdata->blksize);
      data += sizeof(struct LO_eckd_data);
      break;
    case 0x01:
      if (use_prefix)
        {
          prefix(ccw++, (struct PFX_eckd_data *)data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_WRITE_RECORD_ZERO, base, startdev);
          data += sizeof(struct PFX_eckd_data);
        }
      else
        {
          define_extent(ccw++, (struct DE_eckd_data *)data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_WRITE_RECORD_ZERO, startdev, 0);
          data += sizeof(struct DE_eckd_data);
        }
      ccw[-1].flags |= CCW_FLAG_CC;
      locate_record(ccw++, (struct LO_eckd_data *)data, fdata->start_unit, 0, rpt * nr_tracks + 1, DASD_ECKD_CCW_WRITE_RECORD_ZERO, base, base->block->bp_block);
      data += sizeof(struct LO_eckd_data);
      break;
    case 0x04:
      if (use_prefix)
        {
          prefix(ccw++, (struct PFX_eckd_data *)data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_WRITE_CKD, base, startdev);
          data += sizeof(struct PFX_eckd_data);
        }
      else
        {
          define_extent(ccw++, (struct DE_eckd_data *)data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_WRITE_CKD, startdev, 0);
          data += sizeof(struct DE_eckd_data);
        }
      ccw[-1].flags |= CCW_FLAG_CC;
      locate_record(ccw++, (struct LO_eckd_data *)data, fdata->start_unit, 0, 1, DASD_ECKD_CCW_WRITE_CKD, base, 8);
      data += sizeof(struct LO_eckd_data);
      break;
    }
  for (j = 0; j < nr_tracks; j++)
    {

      set_ch_t(&address, (fdata->start_unit + j) / base_priv->rdc_data.trk_per_cyl, (fdata->start_unit + j) % base_priv->rdc_data.trk_per_cyl);
      if (intensity & 0x01)
        {
          ect = (struct eckd_count *)data;
          data += sizeof(struct eckd_count);
          ect->cyl = address.cyl;
          ect->head = address.head;
          ect->record = 0;
          ect->kl = 0;
          ect->dl = 8;
          ccw[-1].flags |= CCW_FLAG_CC;
          ccw->cmd_code = DASD_ECKD_CCW_WRITE_RECORD_ZERO;
          ccw->flags = CCW_FLAG_SLI;
          ccw->count = 8;
          ccw->cda = (__u32)(addr_t)ect;
          ccw++;
        }
      if ((intensity & ~0x08) & 0x04)
        {
          ect = (struct eckd_count *)data;
          data += sizeof(struct eckd_count);
          ect->cyl = address.cyl;
          ect->head = address.head;
          ect->record = 1;
          ect->kl = 0;
          ect->dl = 0;
          ccw[-1].flags |= CCW_FLAG_CC;
          ccw->cmd_code = DASD_ECKD_CCW_WRITE_CKD;
          ccw->flags = CCW_FLAG_SLI;
          ccw->count = 8;
          ccw->cda = (__u32)(addr_t)ect;
        }
      else
        {
          for (i = 0; i < rpt; i++)
            {
              ect = (struct eckd_count *)data;
              data += sizeof(struct eckd_count);
              ect->cyl = address.cyl;
              ect->head = address.head;
              ect->record = i + 1;
              ect->kl = 0;
              ect->dl = fdata->blksize;

              if ((intensity & 0x08) && address.cyl == 0 && address.head == 0)
                {
                  if (i < 3)
                    {
                      ect->kl = 4;
                      ect->dl = sizes_trk0[i] - 4;
                    }
                }
              if ((intensity & 0x08) && address.cyl == 0 && address.head == 1)
                {
                  ect->kl = 44;
                  ect->dl = LABEL_SIZE - 44;
                }
              ccw[-1].flags |= CCW_FLAG_CC;
              if (i != 0 || j == 0)
                ccw->cmd_code = DASD_ECKD_CCW_WRITE_CKD;
              else
                ccw->cmd_code = DASD_ECKD_CCW_WRITE_CKD_MT;
              ccw->flags = CCW_FLAG_SLI;
              ccw->count = 8;
              ccw->cda = (__u32)(addr_t)ect;
              ccw++;
            }
        }
    }
  fcp->startdev = startdev;
  fcp->memdev = startdev;
  fcp->basedev = base;
  fcp->retries = 256;
  fcp->expires = startdev->default_expires * HZ;
  fcp->buildclk = get_tod_clock();
  fcp->status = DASD_CQR_FILLED;
  return fcp;
}