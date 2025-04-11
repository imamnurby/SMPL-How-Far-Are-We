static struct dasd_ccw_req *dasd_eckd_build_check(struct dasd_device *base, struct format_data_t *fdata, int enable_pav, struct eckd_count *fmt_buffer, int rpt)
{
  struct dasd_eckd_private *start_priv;
  struct dasd_eckd_private *base_priv;
  struct dasd_device *startdev = NULL;
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  void *data;
  int cplength, datasize;
  int use_prefix;
  int count;
  int i;
  if (enable_pav)
    startdev = dasd_alias_get_start_dev(base);
  if (!startdev)
    startdev = base;
  start_priv = startdev->private;
  base_priv = base->private;
  count = rpt * (fdata->stop_unit - fdata->start_unit + 1);
  use_prefix = base_priv->features.feature[8] & 0x01;
  if (use_prefix)
    {
      cplength = 1;
      datasize = sizeof(struct PFX_eckd_data);
    }
  else
    {
      cplength = 2;
      datasize = sizeof(struct DE_eckd_data) + sizeof(struct LO_eckd_data);
    }
  cplength += count;
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, cplength, datasize, startdev, NULL);
  if (IS_ERR(cqr))
    return cqr;
  start_priv->count++;
  data = cqr->data;
  ccw = cqr->cpaddr;
  if (use_prefix)
    {
      prefix_LRE(ccw++, data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_READ_COUNT, base, startdev, 1, 0, count, 0, 0);
    }
  else
    {
      define_extent(ccw++, data, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_READ_COUNT, startdev, 0);
      data += sizeof(struct DE_eckd_data);
      ccw[-1].flags |= CCW_FLAG_CC;
      locate_record(ccw++, data, fdata->start_unit, 0, count, DASD_ECKD_CCW_READ_COUNT, base, 0);
    }
  for (i = 0; i < count; i++)
    {
      ccw[-1].flags |= CCW_FLAG_CC;
      ccw->cmd_code = DASD_ECKD_CCW_READ_COUNT;
      ccw->flags = CCW_FLAG_SLI;
      ccw->count = 8;
      ccw->cda = (__u32)(addr_t)fmt_buffer;
      ccw++;
      fmt_buffer++;
    }
  cqr->startdev = startdev;
  cqr->memdev = startdev;
  cqr->basedev = base;
  cqr->retries = DASD_RETRIES;
  cqr->expires = startdev->default_expires * HZ;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;

  set_bit(DASD_CQR_SUPPRESS_NRF, &cqr->flags);
  return cqr;
}