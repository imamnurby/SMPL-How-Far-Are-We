static struct dasd_ccw_req *dasd_eckd_build_check_tcw(struct dasd_device *base, struct format_data_t *fdata, int enable_pav, struct eckd_count *fmt_buffer, int rpt)
{
  struct dasd_eckd_private *start_priv;
  struct dasd_device *startdev = NULL;
  struct tidaw *last_tidaw = NULL;
  struct dasd_ccw_req *cqr;
  struct itcw *itcw;
  int itcw_size;
  int count;
  int rc;
  int i;
  if (enable_pav)
    startdev = dasd_alias_get_start_dev(base);
  if (!startdev)
    startdev = base;
  start_priv = startdev->private;
  count = rpt * (fdata->stop_unit - fdata->start_unit + 1);

  itcw_size = itcw_calc_size(0, count, 0);
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 0, itcw_size, startdev);
  if (IS_ERR(cqr))
    return cqr;
  start_priv->count++;
  itcw = itcw_init(cqr->data, itcw_size, ITCW_OP_READ, 0, count, 0);
  if (IS_ERR(itcw))
    {
      rc = -EINVAL;
      goto out_err;
    }
  cqr->cpaddr = itcw_get_tcw(itcw);
  rc = prepare_itcw(itcw, fdata->start_unit, fdata->stop_unit, DASD_ECKD_CCW_READ_COUNT_MT, base, startdev, 0, count, sizeof(struct eckd_count), count * sizeof(struct eckd_count), 0, rpt);
  if (rc)
    goto out_err;
  for (i = 0; i < count; i++)
    {
      last_tidaw = itcw_add_tidaw(itcw, 0, fmt_buffer++, sizeof(struct eckd_count));
      if (IS_ERR(last_tidaw))
        {
          rc = -EINVAL;
          goto out_err;
        }
    }
  last_tidaw->flags |= TIDAW_FLAGS_LAST;
  itcw_finalize(itcw);
  cqr->cpmode = 1;
  cqr->startdev = startdev;
  cqr->memdev = startdev;
  cqr->basedev = base;
  cqr->retries = startdev->default_retries;
  cqr->expires = startdev->default_expires * HZ;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;

  set_bit(DASD_CQR_SUPPRESS_FP, &cqr->flags);
  set_bit(DASD_CQR_SUPPRESS_IL, &cqr->flags);
  return cqr;
out_err:
  dasd_sfree_request(cqr, startdev);
  return ERR_PTR(rc);
}