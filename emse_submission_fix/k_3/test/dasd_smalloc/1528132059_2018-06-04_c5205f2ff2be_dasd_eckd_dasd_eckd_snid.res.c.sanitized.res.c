static int dasd_eckd_snid(struct dasd_device *device, void __user *argp)
{
  struct dasd_ccw_req *cqr;
  int rc;
  struct ccw1 *ccw;
  int useglobal;
  struct dasd_snid_ioctl_data usrparm;
  if (!capable(CAP_SYS_ADMIN))
    return -EACCES;
  if (copy_from_user(&usrparm, argp, sizeof(usrparm)))
    return -EFAULT;
  useglobal = 0;
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1, sizeof(struct dasd_snid_data), device, NULL);
  if (IS_ERR(cqr))
    {
      mutex_lock(&dasd_reserve_mutex);
      useglobal = 1;
      cqr = &dasd_reserve_req->cqr;
      memset(cqr, 0, sizeof(*cqr));
      memset(&dasd_reserve_req->ccw, 0, sizeof(dasd_reserve_req->ccw));
      cqr->cpaddr = &dasd_reserve_req->ccw;
      cqr->data = &dasd_reserve_req->data;
      cqr->magic = DASD_ECKD_MAGIC;
    }
  ccw = cqr->cpaddr;
  ccw->cmd_code = DASD_ECKD_CCW_SNID;
  ccw->flags |= CCW_FLAG_SLI;
  ccw->count = 12;
  ccw->cda = (__u32)(addr_t)cqr->data;
  cqr->startdev = device;
  cqr->memdev = device;
  clear_bit(DASD_CQR_FLAGS_USE_ERP, &cqr->flags);
  set_bit(DASD_CQR_FLAGS_FAILFAST, &cqr->flags);
  set_bit(DASD_CQR_ALLOW_SLOCK, &cqr->flags);
  cqr->retries = 5;
  cqr->expires = 10 * HZ;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;
  cqr->lpm = usrparm.path_mask;
  rc = dasd_sleep_on_immediatly(cqr);

  if (!rc && usrparm.path_mask && (cqr->lpm != usrparm.path_mask))
    rc = -EIO;
  if (!rc)
    {
      usrparm.data = *((struct dasd_snid_data *)cqr->data);
      if (copy_to_user(argp, &usrparm, sizeof(usrparm)))
        rc = -EFAULT;
    }
  if (useglobal)
    mutex_unlock(&dasd_reserve_mutex);
  else
    dasd_sfree_request(cqr, cqr->memdev);
  return rc;
}