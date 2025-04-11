static struct dasd_ccw_req *
dasd_eckd_analysis_ccw(struct dasd_device *device)
{
	struct dasd_eckd_private *private = device->private;
	struct eckd_count *count_data;
	struct LO_eckd_data *LO_data;
	struct dasd_ccw_req *cqr;
	struct ccw1 *ccw;
	int cplength, datasize;
	int i;
	cplength = 8;
	datasize = sizeof(struct DE_eckd_data) + 2*sizeof(struct LO_eckd_data);
	cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, cplength, datasize,
				   device, NULL);
	if (IS_ERR(cqr))
		return cqr;
	ccw = cqr->cpaddr;
	/* Define extent for the first 3 tracks. */
	define_extent(ccw++, cqr->data, 0, 2,
		      DASD_ECKD_CCW_READ_COUNT, device, 0);
	LO_data = cqr->data + sizeof(struct DE_eckd_data);
	/* Locate record for the first 4 records on track 0. */
	ccw[-1].flags |= CCW_FLAG_CC;
	locate_record(ccw++, LO_data++, 0, 0, 4,
		      DASD_ECKD_CCW_READ_COUNT, device, 0);
	count_data = private->count_area;
	for (i = 0; i < 4; i++) {
		ccw[-1].flags |= CCW_FLAG_CC;
		ccw->cmd_code = DASD_ECKD_CCW_READ_COUNT;
		ccw->flags = 0;
		ccw->count = 8;
		ccw->cda = (__u32)(addr_t) count_data;
		ccw++;
		count_data++;
	}
	/* Locate record for the first record on track 2. */
	ccw[-1].flags |= CCW_FLAG_CC;
	locate_record(ccw++, LO_data++, 2, 0, 1,
		      DASD_ECKD_CCW_READ_COUNT, device, 0);
	/* Read count ccw. */
	ccw[-1].flags |= CCW_FLAG_CC;
	ccw->cmd_code = DASD_ECKD_CCW_READ_COUNT;
	ccw->flags = 0;
	ccw->count = 8;
	ccw->cda = (__u32)(addr_t) count_data;
	cqr->block = NULL;
	cqr->startdev = device;
	cqr->memdev = device;
	cqr->retries = 255;
	cqr->buildclk = get_tod_clock();
	cqr->status = DASD_CQR_FILLED;
	return cqr;
}