static struct dasd_ccw_req *dasd_diag_build_cp(struct dasd_device *memdev, struct dasd_block *block, struct request *req)
{
  struct dasd_ccw_req *cqr;
  struct dasd_diag_req *dreq;
  struct dasd_diag_bio *dbio;
  struct req_iterator iter;
  struct bio_vec bv;
  char *dst;
  unsigned int count, datasize;
  sector_t recid, first_rec, last_rec;
  unsigned int blksize, off;
  unsigned char rw_cmd;
  if (rq_data_dir(req) == READ)
    rw_cmd = MDSK_READ_REQ;
  else if (rq_data_dir(req) == WRITE)
    rw_cmd = MDSK_WRITE_REQ;
  else
    return ERR_PTR(-EINVAL);
  blksize = block->bp_block;

  first_rec = blk_rq_pos(req) >> block->s2b_shift;
  last_rec = (blk_rq_pos(req) + blk_rq_sectors(req) - 1) >> block->s2b_shift;

  count = 0;
  rq_for_each_segment(bv, req, iter)
  {
    if (bv.bv_len & (blksize - 1))

      return ERR_PTR(-EINVAL);
    count += bv.bv_len >> (block->s2b_shift + 9);
  }

  if (count != last_rec - first_rec + 1)
    return ERR_PTR(-EINVAL);

  datasize = sizeof(struct dasd_diag_req) + count * sizeof(struct dasd_diag_bio);
  cqr = dasd_smalloc_request(DASD_DIAG_MAGIC, 0, datasize, memdev, NULL);
  if (IS_ERR(cqr))
    return cqr;
  dreq = (struct dasd_diag_req *)cqr->data;
  dreq->block_count = count;
  dbio = dreq->bio;
  recid = first_rec;
  rq_for_each_segment(bv, req, iter)
  {
    dst = page_address(bv.bv_page) + bv.bv_offset;
    for (off = 0; off < bv.bv_len; off += blksize)
      {
        memset(dbio, 0, sizeof(struct dasd_diag_bio));
        dbio->type = rw_cmd;
        dbio->block_number = recid + 1;
        dbio->buffer = dst;
        dbio++;
        dst += blksize;
        recid++;
      }
  }
  cqr->retries = memdev->default_retries;
  cqr->buildclk = get_tod_clock();
  if (blk_noretry_request(req) || block->base->features & DASD_FEATURE_FAILFAST)
    set_bit(DASD_CQR_FLAGS_FAILFAST, &cqr->flags);
  cqr->startdev = memdev;
  cqr->memdev = memdev;
  cqr->block = block;
  cqr->expires = memdev->default_expires * HZ;
  cqr->status = DASD_CQR_FILLED;
  return cqr;
}