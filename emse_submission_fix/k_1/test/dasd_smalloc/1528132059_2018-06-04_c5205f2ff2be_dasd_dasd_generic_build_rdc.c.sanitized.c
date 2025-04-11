static struct dasd_ccw_req *dasd_generic_build_rdc(struct dasd_device *device, void *rdc_buffer, int rdc_buffer_size, int magic)
{
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  unsigned long *idaw;
  cqr = dasd_smalloc_request(magic, 1, rdc_buffer_size, device);
  if (IS_ERR(cqr))
    {

      dev_err(&device->cdev->dev,
              "An error occurred in the DASD device driver, "
              "reason=%s\n",
              "13");
      return cqr;
    }
  ccw = cqr->cpaddr;
  ccw->cmd_code = CCW_CMD_RDC;
  if (idal_is_needed(rdc_buffer, rdc_buffer_size))
    {
      idaw = (unsigned long *)(cqr->data);
      ccw->cda = (__u32)(addr_t)idaw;
      ccw->flags = CCW_FLAG_IDA;
      idaw = idal_create_words(idaw, rdc_buffer, rdc_buffer_size);
    }
  else
    {
      ccw->cda = (__u32)(addr_t)rdc_buffer;
      ccw->flags = 0;
    }
  ccw->count = rdc_buffer_size;
  cqr->startdev = device;
  cqr->memdev = device;
  cqr->expires = 10 * HZ;
  cqr->retries = 256;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;
  return cqr;
}