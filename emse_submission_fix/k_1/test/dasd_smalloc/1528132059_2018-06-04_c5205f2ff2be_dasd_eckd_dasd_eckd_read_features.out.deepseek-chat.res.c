static int dasd_eckd_read_features(struct dasd_device *device)
{
  struct dasd_eckd_private *private = device->private;
  struct dasd_psf_prssd_data *prssdp;
  struct dasd_rssd_features *features;
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  int rc;
  memset(&private->features, 0, sizeof(struct dasd_rssd_features));
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1 + 1,
                             (sizeof(struct dasd_psf_prssd_data) + sizeof(struct dasd_rssd_features)),
                             device, NULL);
  if (IS_ERR(cqr))
    {
      DBF_EVENT_DEVID(DBF_WARNING,
                      device->cdev,
                      "%s",
                      "Could not "
                      "allocate initialization request");
      return PTR_ERR(cqr);
    }
  cqr->startdev = device;
  cqr->memdev = device;
  cqr->block = NULL;
  cqr->retries = 256;
  cqr->expires = 10 * HZ;

  prssdp = (struct dasd_psf_prssd_data *)cqr->data;
  memset(prssdp, 0, sizeof(struct dasd_psf_prssd_data));
  prssdp->order = PSF_ORDER_PRSSD;
  prssdp->suborder = 0x41;

  ccw = cqr->cpaddr;
  ccw->cmd_code = DASD_ECKD_CCW_PSF;
  ccw->count = sizeof(struct dasd_psf_prssd_data);
  ccw->flags |= CCW_FLAG_CC;
  ccw->cda = (__u32)(addr_t)prssdp;

  features = (struct dasd_rssd_features *)(prssdp + 1);
  memset(features, 0, sizeof(struct dasd_rssd_features));
  ccw++;
  ccw->cmd_code = DASD_ECKD_CCW_RSSD;
  ccw->count = sizeof(struct dasd_rssd_features);
  ccw->cda = (__u32)(addr_t)features;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;
  rc = dasd_sleep_on(cqr);
  if (rc == 0)
    {
      prssdp = (struct dasd_psf_prssd_data *)cqr->data;
      features = (struct dasd_rssd_features *)(prssdp + 1);
      memcpy(&private->features, features, sizeof(struct dasd_rssd_features));
    }
  else
    dev_warn(&device->cdev->dev,
             "Reading device feature codes"
             " failed with rc=%d\n",
             rc);
  dasd_sfree_request(cqr, cqr->memdev);
  return rc;
}