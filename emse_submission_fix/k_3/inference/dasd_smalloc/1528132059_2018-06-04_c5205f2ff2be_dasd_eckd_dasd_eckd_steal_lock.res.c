static int
dasd_eckd_steal_lock(struct dasd_device *device)
{
	struct dasd_ccw_req *cqr;
	int rc;
	struct ccw1 *ccw;
	int useglobal;
	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;
	useglobal = 0;
	cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1, 32, device, NULL);
	if (IS_ERR(cqr)) {
		mutex_lock(&dasd_reserve_mutex);
		useglobal = 1;
		cqr = &dasd_reserve_req->cqr;
		memset(cqr, 0, sizeof(*cqr));
		memset(&dasd_reserve_req->ccw, 0,
		       sizeof(dasd_reserve_req->ccw));
		cqr->cpaddr = &dasd_reserve_req->ccw;
		cqr->data = &dasd_reserve_req->data;
		cqr->magic = DASD_ECKD_MAGIC;
	}
	ccw = cqr->cpaddr;
	ccw->cmd_code = DASD_ECKD_CCW_SLCK;
	ccw->flags |= CCW_FLAG_SLI;
	ccw->count = 32;
	ccw->cda = (__u32)(addr_t) cqr->data;
	cqr->startdev = device;
	cqr->memdev = device;
	clear_bit(DASD_CQR_FLAGS_USE_ERP, &cqr->flags);
	set_bit(DASD_CQR_FLAGS_FAILFAST, &cqr->flags);
	cqr->retries = 2;	/* set retry counter to enable basic ERP */
	cqr->expires = 2 * HZ;
	cqr->buildclk = get_tod_clock();
	cqr->status = DASD_CQR_FILLED;
	rc = dasd_sleep_on_immediatly(cqr);
	if (!rc)
		set_bit(DASD_FLAG_IS_RESERVED, &device->flags);
	if (useglobal)
		mutex_unlock(&dasd_reserve_mutex);
	else
		dasd_sfree_request(cqr, cqr->memdev);
	return rc;
}