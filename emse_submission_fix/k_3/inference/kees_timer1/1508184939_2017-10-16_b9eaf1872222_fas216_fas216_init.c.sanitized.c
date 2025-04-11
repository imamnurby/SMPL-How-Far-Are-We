int fas216_init(struct Scsi_Host *host)
{
  FAS216_Info *info = (FAS216_Info *)host->hostdata;
  info->magic_start = MAGIC;
  info->magic_end = MAGIC;
  info->host = host;
  info->scsi.cfg[0] = host->this_id | CNTL1_PERE;
  info->scsi.cfg[1] = CNTL2_ENF | CNTL2_S2FE;
  info->scsi.cfg[2] = info->ifcfg.cntl3 | CNTL3_ADIDCHK | CNTL3_QTAG | CNTL3_G2CB | CNTL3_LBTM;
  info->scsi.async_stp = fas216_syncperiod(info, info->ifcfg.asyncperiod);
  info->rst_dev_status = -1;
  info->rst_bus_status = -1;
  init_waitqueue_head(&info->eh_wait);
  init_timer(&info->eh_timer);
  info->eh_timer.data = (unsigned long)info;
  info->eh_timer.function = fas216_eh_timer;

  spin_lock_init(&info->host_lock);
  memset(&info->stats, 0, sizeof(info->stats));
  msgqueue_initialise(&info->scsi.msgs);
  if (!queue_initialise(&info->queues.issue))
    return -ENOMEM;
  if (!queue_initialise(&info->queues.disconnected))
    {
      queue_free(&info->queues.issue);
      return -ENOMEM;
    }
  return 0;
}