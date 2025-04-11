static struct dasd_ccw_req *dasd_eckd_build_psf_ssc(struct dasd_device *device,
						    int enable_pav)
{
	struct dasd_ccw_req *cqr;
	struct dasd_psf_ssc_data *psf_ssc_data;
	struct ccw1 *ccw;
	cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1,
				   sizeof(struct dasd_psf_ssc_data), device,
				   NULL);
	if (IS_ERR(cqr)) {
		DBF_DEV_EVENT(DBF_WARNING, device, "%s",
			   "Could not allocate PSF-SSC request");
		return cqr;
	}
	psf_ssc_data = (struct dasd_psf_ssc_data *)cqr->data;
	psf_ssc_data->order = PSF_ORDER_SSC;
	psf_ssc_data->suborder = 0xc0;
	if (enable_pav) {
		psf_ssc_data->suborder |= 0x08;
		psf_ssc_data->reserved[0] = 0x88;
	}
	ccw = cqr->cpaddr;
	ccw->cmd_code = DASD_ECKD_CCW_PSF;
	ccw->cda = (__u32)(addr_t)psf_ssc_data;
	ccw->count = 66;
	cqr->startdev = device;
	cqr->memdev = device;
	cqr->block = NULL;
	cqr->retries = 256;
	cqr->expires = 10*HZ;
	cqr->buildclk = get_tod_clock();
	cqr->status = DASD_CQR_FILLED;
	return cqr;
}