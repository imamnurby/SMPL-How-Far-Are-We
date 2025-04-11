static struct dasd_ccw_req *dasd_fba_build_cp_regular(
						struct dasd_device *memdev,
						struct dasd_block *block,
						struct request *req)
{
	struct dasd_fba_private *private = block->base->private;
	unsigned long *idaws;
	struct LO_fba_data *LO_data;
	struct dasd_ccw_req *cqr;
	struct ccw1 *ccw;
	struct req_iterator iter;
	struct bio_vec bv;
	char *dst;
	int count, cidaw, cplength, datasize;
	sector_t recid, first_rec, last_rec;
	unsigned int blksize, off;
	unsigned char cmd;
	if (rq_data_dir(req) == READ) {
		cmd = DASD_FBA_CCW_READ;
	} else if (rq_data_dir(req) == WRITE) {
		cmd = DASD_FBA_CCW_WRITE;
	} else
		return ERR_PTR(-EINVAL);
	blksize = block->bp_block;
	/* Calculate record id of first and last block. */
	first_rec = blk_rq_pos(req) >> block->s2b_shift;
	last_rec =
		(blk_rq_pos(req) + blk_rq_sectors(req) - 1) >> block->s2b_shift;
	/* Check struct bio and count the number of blocks for the request. */
	count = 0;
	cidaw = 0;
	rq_for_each_segment(bv, req, iter) {
		if (bv.bv_len & (blksize - 1))
			/* Fba can only do full blocks. */
			return ERR_PTR(-EINVAL);
		count += bv.bv_len >> (block->s2b_shift + 9);
		if (idal_is_needed (page_address(bv.bv_page), bv.bv_len))
			cidaw += bv.bv_len / blksize;
	}
	/* Paranoia. */
	if (count != last_rec - first_rec + 1)
		return ERR_PTR(-EINVAL);
	/* 1x define extent + 1x locate record + number of blocks */
	cplength = 2 + count;
	/* 1x define extent + 1x locate record */
	datasize = sizeof(struct DE_fba_data) + sizeof(struct LO_fba_data) +
		cidaw * sizeof(unsigned long);
	/*
	 * Find out number of additional locate record ccws if the device
	 * can't do data chaining.
	 */
	if (private->rdc_data.mode.bits.data_chain == 0) {
		cplength += count - 1;
		datasize += (count - 1)*sizeof(struct LO_fba_data);
	}
	/* Allocate the ccw request. */
	cqr = dasd_smalloc_request(DASD_FBA_MAGIC, cplength, datasize, memdev);
	if (IS_ERR(cqr))
		return cqr;
	ccw = cqr->cpaddr;
	/* First ccw is define extent. */
	define_extent(ccw++, cqr->data, rq_data_dir(req),
		      block->bp_block, blk_rq_pos(req), blk_rq_sectors(req));
	/* Build locate_record + read/write ccws. */
	idaws = (unsigned long *) (cqr->data + sizeof(struct DE_fba_data));
	LO_data = (struct LO_fba_data *) (idaws + cidaw);
	/* Locate record for all blocks for smart devices. */
	if (private->rdc_data.mode.bits.data_chain != 0) {
		ccw[-1].flags |= CCW_FLAG_CC;
		locate_record(ccw++, LO_data++, rq_data_dir(req), 0, count);
	}
	recid = first_rec;
	rq_for_each_segment(bv, req, iter) {
		dst = page_address(bv.bv_page) + bv.bv_offset;
		if (dasd_page_cache) {
			char *copy = kmem_cache_alloc(dasd_page_cache,
						      GFP_DMA | __GFP_NOWARN);
			if (copy && rq_data_dir(req) == WRITE)
				memcpy(copy + bv.bv_offset, dst, bv.bv_len);
			if (copy)
				dst = copy + bv.bv_offset;
		}
		for (off = 0; off < bv.bv_len; off += blksize) {
			/* Locate record for stupid devices. */
			if (private->rdc_data.mode.bits.data_chain == 0) {
				ccw[-1].flags |= CCW_FLAG_CC;
				locate_record(ccw, LO_data++,
					      rq_data_dir(req),
					      recid - first_rec, 1);
				ccw->flags = CCW_FLAG_CC;
				ccw++;
			} else {
				if (recid > first_rec)
					ccw[-1].flags |= CCW_FLAG_DC;
				else
					ccw[-1].flags |= CCW_FLAG_CC;
			}
			ccw->cmd_code = cmd;
			ccw->count = block->bp_block;
			if (idal_is_needed(dst, blksize)) {
				ccw->cda = (__u32)(addr_t) idaws;
				ccw->flags = CCW_FLAG_IDA;
				idaws = idal_create_words(idaws, dst, blksize);
			} else {
				ccw->cda = (__u32)(addr_t) dst;
				ccw->flags = 0;
			}
			ccw++;
			dst += blksize;
			recid++;
		}
	}
	if (blk_noretry_request(req) ||
	    block->base->features & DASD_FEATURE_FAILFAST)
		set_bit(DASD_CQR_FLAGS_FAILFAST, &cqr->flags);
	cqr->startdev = memdev;
	cqr->memdev = memdev;
	cqr->block = block;
	cqr->expires = memdev->default_expires * HZ;	/* default 5 minutes */
	cqr->retries = memdev->default_retries;
	cqr->buildclk = get_tod_clock();
	cqr->status = DASD_CQR_FILLED;
	return cqr;
}