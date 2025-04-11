static int dasd_eckd_performance(struct dasd_device *device, void __user *argp)
{
  struct dasd_psf_prssd_data *prssdp;
  struct dasd_rssd_perf_stats_t *stats;
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  int rc;
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1 + 1, (sizeof(struct dasd_psf_prssd_data) + sizeof(struct dasd_rssd_perf_stats_t)), device, NULL);
  if (IS_ERR(cqr))
    {
      DBF_DEV_EVENT(DBF_WARNING, device, "%s", "Could not allocate initialization request");
      return PTR_ERR(cqr);
    }
  cqr->startdev = device;
  cqr->memdev = device;
  cqr->retries = 0;
  clear_bit(DASD_CQR_FLAGS_USE_ERP, &cqr->flags);
  cqr->expires = 10 * HZ;

  prssdp = (struct dasd_psf_prssd_data *)cqr->data;
  memset(prssdp, 0, sizeof(struct dasd_psf_prssd_data));
  prssdp->order = PSF_ORDER_PRSSD;
  prssdp->suborder = 0x01;
  prssdp->varies[1] = 0x01;
  ccw = cqr->cpaddr;
  ccw->cmd_code = DASD_ECKD_CCW_PSF;
  ccw->count = sizeof(struct dasd_psf_prssd_data);
  ccw->flags |= CCW_FLAG_CC;
  ccw->cda = (__u32)(addr_t)prssdp;

  stats = (struct dasd_rssd_perf_stats_t *)(prssdp + 1);
  memset(stats, 0, sizeof(struct dasd_rssd_perf_stats_t));
  ccw++;
  ccw->cmd_code = DASD_ECKD_CCW_RSSD;
  ccw->count = sizeof(struct dasd_rssd_perf_stats_t);
  ccw->cda = (__u32)(addr_t)stats;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;
  rc = dasd_sleep_on(cqr);
  if (rc == 0)
    {
      prssdp = (struct dasd_psf_prssd_data *)cqr->data;
      stats = (struct dasd_rssd_perf_stats_t *)(prssdp + 1);
      if (copy_to_user(argp, stats, sizeof(struct dasd_rssd_perf_stats_t)))
        rc = -EFAULT;
    }
  dasd_sfree_request(cqr, cqr->memdev);
  return rc;
}