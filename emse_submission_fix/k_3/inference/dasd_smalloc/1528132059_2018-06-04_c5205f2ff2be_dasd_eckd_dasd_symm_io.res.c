static int dasd_symm_io(struct dasd_device *device, void __user *argp)
{
	struct dasd_symmio_parms usrparm;
	char *psf_data, *rssd_result;
	struct dasd_ccw_req *cqr;
	struct ccw1 *ccw;
	char psf0, psf1;
	int rc;
	if (!capable(CAP_SYS_ADMIN) && !capable(CAP_SYS_RAWIO))
		return -EACCES;
	psf0 = psf1 = 0;
	/* Copy parms from caller */
	rc = -EFAULT;
	if (copy_from_user(&usrparm, argp, sizeof(usrparm)))
		goto out;
	if (is_compat_task()) {
		/* Make sure pointers are sane even on 31 bit. */
		rc = -EINVAL;
		if ((usrparm.psf_data >> 32) != 0)
			goto out;
		if ((usrparm.rssd_result >> 32) != 0)
			goto out;
		usrparm.psf_data &= 0x7fffffffULL;
		usrparm.rssd_result &= 0x7fffffffULL;
	}
	/* alloc I/O data area */
	psf_data = kzalloc(usrparm.psf_data_len, GFP_KERNEL | GFP_DMA);
	rssd_result = kzalloc(usrparm.rssd_result_len, GFP_KERNEL | GFP_DMA);
	if (!psf_data || !rssd_result) {
		rc = -ENOMEM;
		goto out_free;
	}
	/* get syscall header from user space */
	rc = -EFAULT;
	if (copy_from_user(psf_data,
			   (void __user *)(unsigned long) usrparm.psf_data,
			   usrparm.psf_data_len))
		goto out_free;
	psf0 = psf_data[0];
	psf1 = psf_data[1];
	/* setup CCWs for PSF + RSSD */
	cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 2, 0, device, NULL);
	if (IS_ERR(cqr)) {
		DBF_DEV_EVENT(DBF_WARNING, device, "%s",
			"Could not allocate initialization request");
		rc = PTR_ERR(cqr);
		goto out_free;
	}
	cqr->startdev = device;
	cqr->memdev = device;
	cqr->retries = 3;
	cqr->expires = 10 * HZ;
	cqr->buildclk = get_tod_clock();
	cqr->status = DASD_CQR_FILLED;
	/* Build the ccws */
	ccw = cqr->cpaddr;
	/* PSF ccw */
	ccw->cmd_code = DASD_ECKD_CCW_PSF;
	ccw->count = usrparm.psf_data_len;
	ccw->flags |= CCW_FLAG_CC;
	ccw->cda = (__u32)(addr_t) psf_data;
	ccw++;
	/* RSSD ccw  */
	ccw->cmd_code = DASD_ECKD_CCW_RSSD;
	ccw->count = usrparm.rssd_result_len;
	ccw->flags = CCW_FLAG_SLI ;
	ccw->cda = (__u32)(addr_t) rssd_result;
	rc = dasd_sleep_on(cqr);
	if (rc)
		goto out_sfree;
	rc = -EFAULT;
	if (copy_to_user((void __user *)(unsigned long) usrparm.rssd_result,
			   rssd_result, usrparm.rssd_result_len))
		goto out_sfree;
	rc = 0;
out_sfree:
	dasd_sfree_request(cqr, cqr->memdev);
out_free:
	kfree(rssd_result);
	kfree(psf_data);
out:
	DBF_DEV_EVENT(DBF_WARNING, device,
		      "Symmetrix ioctl (0x%02x 0x%02x): rc=%d",
		      (int) psf0, (int) psf1, rc);
	return rc;
}