static struct dasd_ccw_req *dasd_eckd_build_cp_cmd_track(struct dasd_device *startdev, struct dasd_block *block, struct request *req, sector_t first_rec, sector_t last_rec, sector_t first_trk, sector_t last_trk, unsigned int first_offs, unsigned int last_offs, unsigned int blk_per_trk, unsigned int blksize)
{
  unsigned long *idaws;
  struct dasd_ccw_req *cqr;
  struct ccw1 *ccw;
  struct req_iterator iter;
  struct bio_vec bv;
  char *dst, *idaw_dst;
  unsigned int cidaw, cplength, datasize;
  unsigned int tlf;
  sector_t recid;
  unsigned char cmd;
  struct dasd_device *basedev;
  unsigned int trkcount, count, count_to_trk_end;
  unsigned int idaw_len, seg_len, part_len, len_to_track_end;
  unsigned char new_track, end_idaw;
  sector_t trkid;
  unsigned int recoffs;
  basedev = block->base;
  if (rq_data_dir(req) == READ)
    cmd = DASD_ECKD_CCW_READ_TRACK_DATA;
  else if (rq_data_dir(req) == WRITE)
    cmd = DASD_ECKD_CCW_WRITE_TRACK_DATA;
  else
    return ERR_PTR(-EINVAL);

  cidaw = last_rec - first_rec + 1;
  trkcount = last_trk - first_trk + 1;

  cplength = 1 + trkcount;
  datasize = sizeof(struct PFX_eckd_data) + cidaw * sizeof(unsigned long);

  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, cplength, datasize, startdev, blk_mq_rq_to_pdu(req));
  if (IS_ERR(cqr))
    return cqr;
  ccw = cqr->cpaddr;

  if (first_trk == last_trk)
    tlf = last_offs - first_offs + 1;
  else
    tlf = last_offs + 1;
  tlf *= blksize;
  if (prefix_LRE(ccw++, cqr->data, first_trk, last_trk, cmd, basedev, startdev, 1, first_offs + 1, trkcount, blksize, tlf) == -EAGAIN)
    {

      dasd_sfree_request(cqr, startdev);
      return ERR_PTR(-EAGAIN);
    }

  idaws = (unsigned long *)(cqr->data + sizeof(struct PFX_eckd_data));
  recid = first_rec;
  new_track = 1;
  end_idaw = 0;
  len_to_track_end = 0;
  idaw_dst = NULL;
  idaw_len = 0;
  rq_for_each_segment(bv, req, iter)
  {
    dst = page_address(bv.bv_page) + bv.bv_offset;
    seg_len = bv.bv_len;
    while (seg_len)
      {
        if (new_track)
          {
            trkid = recid;
            recoffs = sector_div(trkid, blk_per_trk);
            count_to_trk_end = blk_per_trk - recoffs;
            count = min((last_rec - recid + 1), (sector_t)count_to_trk_end);
            len_to_track_end = count * blksize;
            ccw[-1].flags |= CCW_FLAG_CC;
            ccw->cmd_code = cmd;
            ccw->count = len_to_track_end;
            ccw->cda = (__u32)(addr_t)idaws;
            ccw->flags = CCW_FLAG_IDA;
            ccw++;
            recid += count;
            new_track = 0;

            if (!idaw_dst)
              idaw_dst = dst;
          }

        if (!idaw_dst)
          {
            if (__pa(dst) & (IDA_BLOCK_SIZE - 1))
              {
                dasd_sfree_request(cqr, startdev);
                return ERR_PTR(-ERANGE);
              }
            else
              idaw_dst = dst;
          }
        if ((idaw_dst + idaw_len) != dst)
          {
            dasd_sfree_request(cqr, startdev);
            return ERR_PTR(-ERANGE);
          }
        part_len = min(seg_len, len_to_track_end);
        seg_len -= part_len;
        dst += part_len;
        idaw_len += part_len;
        len_to_track_end -= part_len;

        if (!(__pa(idaw_dst + idaw_len) & (IDA_BLOCK_SIZE - 1)))
          end_idaw = 1;

        if (!len_to_track_end)
          {
            new_track = 1;
            end_idaw = 1;
          }
        if (end_idaw)
          {
            idaws = idal_create_words(idaws, idaw_dst, idaw_len);
            idaw_dst = NULL;
            idaw_len = 0;
            end_idaw = 0;
          }
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