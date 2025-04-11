static struct dasd_ccw_req *dasd_eckd_build_cp_raw(struct dasd_device *startdev,
						   struct dasd_block *block,
						   struct request *req)
{
	sector_t start_padding_sectors, end_sector_offset, end_padding_sectors;
	unsigned int seg_len, len_to_track_end;
	unsigned int cidaw, cplength, datasize;
	sector_t first_trk, last_trk, sectors;
	struct dasd_eckd_private *base_priv;
	struct dasd_device *basedev;
	struct req_iterator iter;
	struct dasd_ccw_req *cqr;
	unsigned int first_offs;
	unsigned int trkcount;
	unsigned long *idaws;
	unsigned int size;
	unsigned char cmd;
	struct bio_vec bv;
	struct ccw1 *ccw;
	int use_prefix;
	void *data;
	char *dst;
	/*
	 * raw track access needs to be mutiple of 64k and on 64k boundary
	 * For read requests we can fix an incorrect alignment by padding
	 * the request with dummy pages.
	 */
	start_padding_sectors = blk_rq_pos(req) % DASD_RAW_SECTORS_PER_TRACK;
	end_sector_offset = (blk_rq_pos(req) + blk_rq_sectors(req)) %
		DASD_RAW_SECTORS_PER_TRACK;
	end_padding_sectors = (DASD_RAW_SECTORS_PER_TRACK - end_sector_offset) %
		DASD_RAW_SECTORS_PER_TRACK;
	basedev = block->base;
	if ((start_padding_sectors || end_padding_sectors) &&
	    (rq_data_dir(req) == WRITE)) {
		DBF_DEV_EVENT(DBF_ERR, basedev,
			      "raw write not track aligned (%lu,%lu) req %p",
			      start_padding_sectors, end_padding_sectors, req);
		return ERR_PTR(-EINVAL);
	}
	first_trk = blk_rq_pos(req) / DASD_RAW_SECTORS_PER_TRACK;
	last_trk = (blk_rq_pos(req) + blk_rq_sectors(req) - 1) /
		DASD_RAW_SECTORS_PER_TRACK;
	trkcount = last_trk - first_trk + 1;
	first_offs = 0;
	if (rq_data_dir(req) == READ)
		cmd = DASD_ECKD_CCW_READ_TRACK;
	else if (rq_data_dir(req) == WRITE)
		cmd = DASD_ECKD_CCW_WRITE_FULL_TRACK;
	else
		return ERR_PTR(-EINVAL);
	/*
	 * Raw track based I/O needs IDAWs for each page,
	 * and not just for 64 bit addresses.
	 */
	cidaw = trkcount * DASD_RAW_BLOCK_PER_TRACK;
	/*
	 * struct PFX_eckd_data and struct LRE_eckd_data can have up to 2 bytes
	 * of extended parameter. This is needed for write full track.
	 */
	base_priv = basedev->private;
	use_prefix = base_priv->features.feature[8] & 0x01;
	if (use_prefix) {
		cplength = 1 + trkcount;
		size = sizeof(struct PFX_eckd_data) + 2;
	} else {
		cplength = 2 + trkcount;
		size = sizeof(struct DE_eckd_data) +
			sizeof(struct LRE_eckd_data) + 2;
	}
	size = ALIGN(size, 8);
	datasize = size + cidaw * sizeof(unsigned long);
	/* Allocate the ccw request. */
	cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, cplength,
				   datasize, startdev);
	if (IS_ERR(cqr))
		return cqr;
	ccw = cqr->cpaddr;
	data = cqr->data;
	if (use_prefix) {
		prefix_LRE(ccw++, data, first_trk, last_trk, cmd, basedev,
			   startdev, 1, first_offs + 1, trkcount, 0, 0);
	} else {
		define_extent(ccw++, data, first_trk, last_trk, cmd, basedev, 0);
		ccw[-1].flags |= CCW_FLAG_CC;
		data += sizeof(struct DE_eckd_data);
		locate_record_ext(ccw++, data, first_trk, first_offs + 1,
				  trkcount, cmd, basedev, 0, 0);
	}
	idaws = (unsigned long *)(cqr->data + size);
	len_to_track_end = 0;
	if (start_padding_sectors) {
		ccw[-1].flags |= CCW_FLAG_CC;
		ccw->cmd_code = cmd;
		/* maximum 3390 track size */
		ccw->count = 57326;
		/* 64k map to one track */
		len_to_track_end = 65536 - start_padding_sectors * 512;
		ccw->cda = (__u32)(addr_t)idaws;
		ccw->flags |= CCW_FLAG_IDA;
		ccw->flags |= CCW_FLAG_SLI;
		ccw++;
		for (sectors = 0; sectors < start_padding_sectors; sectors += 8)
			idaws = idal_create_words(idaws, rawpadpage, PAGE_SIZE);
	}
	rq_for_each_segment(bv, req, iter) {
		dst = page_address(bv.bv_page) + bv.bv_offset;
		seg_len = bv.bv_len;
		if (cmd == DASD_ECKD_CCW_READ_TRACK)
			memset(dst, 0, seg_len);
		if (!len_to_track_end) {
			ccw[-1].flags |= CCW_FLAG_CC;
			ccw->cmd_code = cmd;
			/* maximum 3390 track size */
			ccw->count = 57326;
			/* 64k map to one track */
			len_to_track_end = 65536;
			ccw->cda = (__u32)(addr_t)idaws;
			ccw->flags |= CCW_FLAG_IDA;
			ccw->flags |= CCW_FLAG_SLI;
			ccw++;
		}
		len_to_track_end -= seg_len;
		idaws = idal_create_words(idaws, dst, seg_len);
	}
	for (sectors = 0; sectors < end_padding_sectors; sectors += 8)
		idaws = idal_create_words(idaws, rawpadpage, PAGE_SIZE);
	if (blk_noretry_request(req) ||
	    block->base->features & DASD_FEATURE_FAILFAST)
		set_bit(DASD_CQR_FLAGS_FAILFAST, &cqr->flags);
	cqr->startdev = startdev;
	cqr->memdev = startdev;
	cqr->block = block;
	cqr->expires = startdev->default_expires * HZ;
	cqr->lpm = dasd_path_get_ppm(startdev);
	cqr->retries = startdev->default_retries;
	cqr->buildclk = get_tod_clock();
	cqr->status = DASD_CQR_FILLED;
	return cqr;
}