static struct dasd_ccw_req *dasd_eckd_build_cp_cmd_single(struct dasd_device *startdev, struct dasd_block *block, struct request *req, sector_t first_rec, sector_t last_rec, sector_t first_trk, sector_t last_trk, unsigned int first_offs, unsigned int last_offs, unsigned int blk_per_trk, unsigned int blksize)
{
  struct dasd_eckd_private *private;
  unsigned long *idaws;
  struct LO_eckd_data *LO_data;
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  struct req_iterator iter;
  struct bio_vec bv;
  char *dst;
  unsigned int off;
  int count, cidaw, cplength, datasize;
  sector_t recid;
  unsigned char cmd, rcmd;
  int use_prefix;
  struct dasd_device *basedev;
  basedev = block->base;
private
  = basedev->private;
  if (rq_data_dir(req) == READ)
    cmd = DASD_ECKD_CCW_READ_MT;
  else if (rq_data_dir(req) == WRITE)
    cmd = DASD_ECKD_CCW_WRITE_MT;
  else
    return ERR_PTR(-EINVAL);

  count = 0;
  cidaw = 0;
  rq_for_each_segment(bv, req, iter)
  {
    if (bv.bv_len & (blksize - 1))

      return ERR_PTR(-EINVAL);
    count += bv.bv_len >> (block->s2b_shift + 9);
    if (idal_is_needed(page_address(bv.bv_page), bv.bv_len))
      cidaw += bv.bv_len >> (block->s2b_shift + 9);
  }

  if (count != last_rec - first_rec + 1)
    return ERR_PTR(-EINVAL);

  use_prefix = private->features.feature[8] & 0x01;
  if (use_prefix)
    {

      cplength = 2 + count;

      datasize = sizeof(struct PFX_eckd_data) + sizeof(struct LO_eckd_data) + cidaw * sizeof(unsigned long);
    }
  else
    {

      cplength = 2 + count;

      datasize = sizeof(struct DE_eckd_data) + sizeof(struct LO_eckd_data) + cidaw * sizeof(unsigned long);
    }

  if (private->uses_cdl && first_rec < 2 * blk_per_trk)
    {
      if (last_rec >= 2 * blk_per_trk)
        count = 2 * blk_per_trk - first_rec;
      cplength += count;
      datasize += count * sizeof(struct LO_eckd_data);
    }

  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, cplength, datasize, startdev, blk_mq_rq_to_pdu(req));
  if (IS_ERR(cqr))
    return cqr;
  ccw = cqr->cpaddr;

  if (use_prefix)
    {
      if (prefix(ccw++, cqr->data, first_trk, last_trk, cmd, basedev, startdev) == -EAGAIN)
        {

          dasd_sfree_request(cqr, startdev);
          return ERR_PTR(-EAGAIN);
        }
      idaws = (unsigned long *)(cqr->data + sizeof(struct PFX_eckd_data));
    }
  else
    {
      if (define_extent(ccw++, cqr->data, first_trk, last_trk, cmd, basedev, 0) == -EAGAIN)
        {

          dasd_sfree_request(cqr, startdev);
          return ERR_PTR(-EAGAIN);
        }
      idaws = (unsigned long *)(cqr->data + sizeof(struct DE_eckd_data));
    }

  LO_data = (struct LO_eckd_data *)(idaws + cidaw);
  recid = first_rec;
  if (private->uses_cdl == 0 || recid > 2 * blk_per_trk)
    {

      ccw[-1].flags |= CCW_FLAG_CC;
      locate_record(ccw++, LO_data++, first_trk, first_offs + 1, last_rec - recid + 1, cmd, basedev, blksize);
    }
  rq_for_each_segment(bv, req, iter)
  {
    dst = page_address(bv.bv_page) + bv.bv_offset;
    if (dasd_page_cache)
      {
        char *copy = kmem_cache_alloc(dasd_page_cache, GFP_DMA | __GFP_NOWARN);
        if (copy && rq_data_dir(req) == WRITE)
          memcpy(copy + bv.bv_offset, dst, bv.bv_len);
        if (copy)
          dst = copy + bv.bv_offset;
      }
    for (off = 0; off < bv.bv_len; off += blksize)
      {
        sector_t trkid = recid;
        unsigned int recoffs = sector_div(trkid, blk_per_trk);
        rcmd = cmd;
        count = blksize;

        if (private->uses_cdl && recid < 2 * blk_per_trk)
          {
            if (dasd_eckd_cdl_special(blk_per_trk, recid))
              {
                rcmd |= 0x8;
                count = dasd_eckd_cdl_reclen(recid);
                if (count < blksize && rq_data_dir(req) == READ)
                  memset(dst + count, 0xe5, blksize - count);
              }
            ccw[-1].flags |= CCW_FLAG_CC;
            locate_record(ccw++, LO_data++, trkid, recoffs + 1, 1, rcmd, basedev, count);
          }

        if (private->uses_cdl && recid == 2 * blk_per_trk)
          {
            ccw[-1].flags |= CCW_FLAG_CC;
            locate_record(ccw++, LO_data++, trkid, recoffs + 1, last_rec - recid + 1, cmd, basedev, count);
          }

        ccw[-1].flags |= CCW_FLAG_CC;
        ccw->cmd_code = rcmd;
        ccw->count = count;
        if (idal_is_needed(dst, blksize))
          {
            ccw->cda = (__u32)(addr_t)idaws;
            ccw->flags = CCW_FLAG_IDA;
            idaws = idal_create_words(idaws, dst, blksize);
          }
        else
          {
            ccw->cda = (__u32)(addr_t)dst;
            ccw->flags = 0;
          }
        ccw++;
        dst += blksize;
        recid++;
      }
  }
  if (blk_noretry_request(req) || block->base->features & DASD_FEATURE_FAILFAST)
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