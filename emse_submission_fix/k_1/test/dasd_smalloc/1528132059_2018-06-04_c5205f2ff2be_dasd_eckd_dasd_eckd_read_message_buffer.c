static int dasd_eckd_read_message_buffer(struct dasd_device *device,
					 struct dasd_rssd_messages *messages,
					 __u8 lpum)
{
	struct dasd_rssd_messages *message_buf;
	struct dasd_psf_prssd_data *prssdp;
	struct dasd_ccw_req *cqr;
	struct ccw1 *ccw;
	int rc;
	cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1 /* PSF */	+ 1 /* RSSD */,
				   (sizeof(struct dasd_psf_prssd_data) +
				    sizeof(struct dasd_rssd_messages)),
				   device);
	if (IS_ERR(cqr)) {
		DBF_EVENT_DEVID(DBF_WARNING, device->cdev, "%s",
				"Could not allocate read message buffer request");
		return PTR_ERR(cqr);
	}
	cqr->lpm = lpum;
retry:
	cqr->startdev = device;
	cqr->memdev = device;
	cqr->block = NULL;
	cqr->expires = 10 * HZ;
	set_bit(DASD_CQR_VERIFY_PATH, &cqr->flags);
	/* dasd_sleep_on_immediatly does not do complex error
	 * recovery so clear erp flag and set retry counter to
	 * do basic erp */
	clear_bit(DASD_CQR_FLAGS_USE_ERP, &cqr->flags);
	cqr->retries = 256;
	/* Prepare for Read Subsystem Data */
	prssdp = (struct dasd_psf_prssd_data *) cqr->data;
	memset(prssdp, 0, sizeof(struct dasd_psf_prssd_data));
	prssdp->order = PSF_ORDER_PRSSD;
	prssdp->suborder = 0x03;	/* Message Buffer */
	/* all other bytes of prssdp must be zero */
	ccw = cqr->cpaddr;
	ccw->cmd_code = DASD_ECKD_CCW_PSF;
	ccw->count = sizeof(struct dasd_psf_prssd_data);
	ccw->flags |= CCW_FLAG_CC;
	ccw->flags |= CCW_FLAG_SLI;
	ccw->cda = (__u32)(addr_t) prssdp;
	/* Read Subsystem Data - message buffer */
	message_buf = (struct dasd_rssd_messages *) (prssdp + 1);
	memset(message_buf, 0, sizeof(struct dasd_rssd_messages));
	ccw++;
	ccw->cmd_code = DASD_ECKD_CCW_RSSD;
	ccw->count = sizeof(struct dasd_rssd_messages);
	ccw->flags |= CCW_FLAG_SLI;
	ccw->cda = (__u32)(addr_t) message_buf;
	cqr->buildclk = get_tod_clock();
	cqr->status = DASD_CQR_FILLED;
	rc = dasd_sleep_on_immediatly(cqr);
	if (rc == 0) {
		prssdp = (struct dasd_psf_prssd_data *) cqr->data;
		message_buf = (struct dasd_rssd_messages *)
			(prssdp + 1);
		memcpy(messages, message_buf,
		       sizeof(struct dasd_rssd_messages));
	} else if (cqr->lpm) {
		/*
		 * on z/VM we might not be able to do I/O on the requested path
		 * but instead we get the required information on any path
		 * so retry with open path mask
		 */
		cqr->lpm = 0;
		goto retry;
	} else
		DBF_EVENT_DEVID(DBF_WARNING, device->cdev,
				"Reading messages failed with rc=%d\n"
				, rc);
	dasd_sfree_request(cqr, cqr->memdev);
	return rc;
}