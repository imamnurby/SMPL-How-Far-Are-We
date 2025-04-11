static int dasd_eckd_query_host_access(struct dasd_device *device, struct dasd_psf_query_host_access *data)
{
  struct dasd_eckd_private *private = device->private;
  struct dasd_psf_query_host_access *host_access;
  struct dasd_psf_prssd_data *prssdp;
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  int rc;

  if (!device->block && private->lcu->pav == HYPER_PAV)
    return -EOPNOTSUPP;

  if (!(private->features.feature[14] & 0x80))
    return -EOPNOTSUPP;
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1 + 1, sizeof(struct dasd_psf_prssd_data) + 1, device, NULL);
  if (IS_ERR(cqr))
    {
      DBF_EVENT_DEVID(DBF_WARNING, device->cdev, "%s", "Could not allocate read message buffer request");
      return PTR_ERR(cqr);
    }
  host_access = kzalloc(sizeof(*host_access), GFP_KERNEL | GFP_DMA);
  if (!host_access)
    {
      dasd_sfree_request(cqr, device);
      DBF_EVENT_DEVID(DBF_WARNING, device->cdev, "%s", "Could not allocate host_access buffer");
      return -ENOMEM;
    }
  cqr->startdev = device;
  cqr->memdev = device;
  cqr->block = NULL;
  cqr->retries = 256;
  cqr->expires = 10 * HZ;

  prssdp = (struct dasd_psf_prssd_data *)cqr->data;
  memset(prssdp, 0, sizeof(struct dasd_psf_prssd_data));
  prssdp->order = PSF_ORDER_PRSSD;
  prssdp->suborder = PSF_SUBORDER_QHA;

  prssdp->lss = private->ned->ID;
  prssdp->volume = private->ned->unit_addr;

  ccw = cqr->cpaddr;
  ccw->cmd_code = DASD_ECKD_CCW_PSF;
  ccw->count = sizeof(struct dasd_psf_prssd_data);
  ccw->flags |= CCW_FLAG_CC;
  ccw->flags |= CCW_FLAG_SLI;
  ccw->cda = (__u32)(addr_t)prssdp;

  ccw++;
  ccw->cmd_code = DASD_ECKD_CCW_RSSD;
  ccw->count = sizeof(struct dasd_psf_query_host_access);
  ccw->flags |= CCW_FLAG_SLI;
  ccw->cda = (__u32)(addr_t)host_access;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;

  __set_bit(DASD_CQR_SUPPRESS_CR, &cqr->flags);
  rc = dasd_sleep_on_interruptible(cqr);
  if (rc == 0)
    {
      *data = *host_access;
    }
  else
    {
      DBF_EVENT_DEVID(DBF_WARNING, device->cdev, "Reading host access data failed with rc=%d\n", rc);
      rc = -EOPNOTSUPP;
    }
  dasd_sfree_request(cqr, cqr->memdev);
  kfree(host_access);
  return rc;
}