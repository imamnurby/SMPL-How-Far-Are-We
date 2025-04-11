static int drr_init_qdisc(struct Qdisc *sch, struct nlattr *opt, struct netlink_ext_ack *extack)
{
  struct drr_sched *q = qdisc_priv(sch);
  int err;
  err = tcf_block_get(&q->block, &q->filter_list, sch);
  if (err)
    return err;
  err = qdisc_class_hash_init(&q->clhash);
  if (err < 0)
    return err;
  INIT_LIST_HEAD(&q->active);
  return 0;
}