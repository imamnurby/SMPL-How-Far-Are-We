static struct dasd_ccw_req *dasd_eckd_build_cp_tpm_track(struct dasd_device *startdev, struct dasd_block *block, struct request *req, sector_t first_rec, sector_t last_rec, sector_t first_trk, sector_t last_trk, unsigned int first_offs, unsigned int last_offs, unsigned int blk_per_trk, unsigned int blksize)
{
  struct dasd_ccw_req *cqr;
  struct req_iterator iter;
  struct bio_vec bv;
  char *dst;
  unsigned int trkcount, ctidaw;
  unsigned char cmd;
  struct dasd_device *basedev;
  unsigned int tlf;
  struct itcw *itcw;
  struct tidaw *last_tidaw = NULL;
  int itcw_op;
  size_t itcw_size;
  u8 tidaw_flags;
  unsigned int seg_len, part_len, len_to_track_end;
  unsigned char new_track;
  sector_t recid, trkid;
  unsigned int offs;
  unsigned int count, count_to_trk_end;
  int ret;
  basedev = block->base;
  if (rq_data_dir(req) == READ)
    {
      cmd = DASD_ECKD_CCW_READ_TRACK_DATA;
      itcw_op = ITCW_OP_READ;
    }
  else if (rq_data_dir(req) == WRITE)
    {
      cmd = DASD_ECKD_CCW_WRITE_TRACK_DATA;
      itcw_op = ITCW_OP_WRITE;
    }
  else
    return ERR_PTR(-EINVAL);

  trkcount = last_trk - first_trk + 1;
  ctidaw = 0;
  rq_for_each_segment(bv, req, iter) { ++ctidaw; }
  if (rq_data_dir(req) == WRITE)
    ctidaw += (last_trk - first_trk);

  itcw_size = itcw_calc_size(0, ctidaw, 0);
  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 0, itcw_size, startdev, NULL);
  if (IS_ERR(cqr))
    return cqr;

  if (first_trk == last_trk)
    tlf = last_offs - first_offs + 1;
  else
    tlf = last_offs + 1;
  tlf *= blksize;
  itcw = itcw_init(cqr->data, itcw_size, itcw_op, 0, ctidaw, 0);
  if (IS_ERR(itcw))
    {
      ret = -EINVAL;
      goto out_error;
    }
  cqr->cpaddr = itcw_get_tcw(itcw);
  if (prepare_itcw(itcw, first_trk, last_trk, cmd, basedev, startdev, first_offs + 1, trkcount, blksize, (last_rec - first_rec + 1) * blksize, tlf, blk_per_trk) == -EAGAIN)
    {

      ret = -EAGAIN;
      goto out_error;
    }
  len_to_track_end = 0;

  if (rq_data_dir(req) == WRITE)
    {
      new_track = 1;
      recid = first_rec;
      rq_for_each_segment(bv, req, iter)
      {
        dst = page_address(bv.bv_page) + bv.bv_offset;
        seg_len = bv.bv_len;
        while (seg_len)
          {
            if (new_track)
              {
                trkid = recid;
                offs = sector_div(trkid, blk_per_trk);
                count_to_trk_end = blk_per_trk - offs;
                count = min((last_rec - recid + 1), (sector_t)count_to_trk_end);
                len_to_track_end = count * blksize;
                recid += count;
                new_track = 0;
              }
            part_len = min(seg_len, len_to_track_end);
            seg_len -= part_len;
            len_to_track_end -= part_len;

            if (!len_to_track_end)
              {
                new_track = 1;
                tidaw_flags = TIDAW_FLAGS_INSERT_CBC;
              }
            else
              tidaw_flags = 0;
            last_tidaw = itcw_add_tidaw(itcw, tidaw_flags, dst, part_len);
            if (IS_ERR(last_tidaw))
              {
                ret = -EINVAL;
                goto out_error;
              }
            dst += part_len;
          }
      }
    }
  else
    {
      rq_for_each_segment(bv, req, iter)
      {
        dst = page_address(bv.bv_page) + bv.bv_offset;
        last_tidaw = itcw_add_tidaw(itcw, 0x00, dst, bv.bv_len);
        if (IS_ERR(last_tidaw))
          {
            ret = -EINVAL;
            goto out_error;
          }
      }
    }
  last_tidaw->flags |= TIDAW_FLAGS_LAST;
  last_tidaw->flags &= ~TIDAW_FLAGS_INSERT_CBC;
  itcw_finalize(itcw);
  if (blk_noretry_request(req) || block->base->features & DASD_FEATURE_FAILFAST)
    set_bit(DASD_CQR_FLAGS_FAILFAST, &cqr->flags);
  cqr->cpmode = 1;
  cqr->startdev = startdev;
  cqr->memdev = startdev;
  cqr->block = block;
  cqr->expires = startdev->default_expires * HZ;
  cqr->lpm = dasd_path_get_ppm(startdev);
  cqr->retries = startdev->default_retries;
  cqr->buildclk = get_tod_clock();
  cqr->status = DASD_CQR_FILLED;
  return cqr;
out_error:
  dasd_sfree_request(cqr, startdev);
  return ERR_PTR(ret);
}