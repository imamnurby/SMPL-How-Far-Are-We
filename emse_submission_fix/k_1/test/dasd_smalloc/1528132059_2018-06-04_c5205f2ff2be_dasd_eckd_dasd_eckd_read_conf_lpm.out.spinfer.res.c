static int dasd_eckd_read_conf_lpm(struct dasd_device *device, void **rcd_buffer, int *rcd_buffer_size, __u8 lpm)
{
  struct ciw *ciw;
  char *rcd_buf = NULL;
  int ret;
  struct dasd_ccw_req *cqr;

  ciw = ccw_device_get_ciw(device->cdev, CIW_TYPE_RCD);
  if (!ciw || ciw->cmd != DASD_ECKD_CCW_RCD)
    {
      ret = -EOPNOTSUPP;
      goto out_error;
    }
  rcd_buf = kzalloc(DASD_ECKD_RCD_DATA_SIZE, GFP_KERNEL | GFP_DMA);
  if (!rcd_buf)
    {
      ret = -ENOMEM;
      goto out_error;
    }
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1, 0, device, NULL);
  if (IS_ERR(cqr))
    {
      DBF_DEV_EVENT(DBF_WARNING, device, "%s", "Could not allocate RCD request");
      ret = -ENOMEM;
      goto out_error;
    }
  dasd_eckd_fill_rcd_cqr(device, cqr, rcd_buf, lpm);
  cqr->callback = read_conf_cb;
  ret = dasd_sleep_on(cqr);

  dasd_sfree_request(cqr, cqr->memdev);
  if (ret)
    goto out_error;
  *rcd_buffer_size = DASD_ECKD_RCD_DATA_SIZE;
  *rcd_buffer = rcd_buf;
  return 0;
out_error:
  kfree(rcd_buf);
  *rcd_buffer = NULL;
  *rcd_buffer_size = 0;
  return ret;
}