static int dasd_eckd_psf_cuir_response(struct dasd_device *device, int response, __u32 message_id, __u8 lpum)
{
  struct dasd_psf_cuir_response *psf_cuir;
  int pos = pathmask_to_pos(lpum);
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  int rc;
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1, sizeof(struct dasd_psf_cuir_response), device, NULL);
  if (IS_ERR(cqr))
    {
      DBF_DEV_EVENT(DBF_WARNING, device, "%s", "Could not allocate PSF-CUIR request");
      return PTR_ERR(cqr);
    }
  psf_cuir = (struct dasd_psf_cuir_response *)cqr->data;
  psf_cuir->order = PSF_ORDER_CUIR_RESPONSE;
  psf_cuir->cc = response;
  psf_cuir->chpid = device->path[pos].chpid;
  psf_cuir->message_id = message_id;
  psf_cuir->cssid = device->path[pos].cssid;
  psf_cuir->ssid = device->path[pos].ssid;
  ccw = cqr->cpaddr;
  ccw->cmd_code = DASD_ECKD_CCW_PSF;
  ccw->cda = (__u32)(addr_t)psf_cuir;
  ccw->flags = CCW_FLAG_SLI;
  ccw->count = sizeof(struct dasd_psf_cuir_response);
  cqr->startdev = device;
  cqr->memdev = device;
  cqr->block = NULL;
  cqr->retries = 256;
  cqr->expires = 10 * HZ;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;
  set_bit(DASD_CQR_VERIFY_PATH, &cqr->flags);
  rc = dasd_sleep_on(cqr);
  dasd_sfree_request(cqr, cqr->memdev);
  return rc;
}