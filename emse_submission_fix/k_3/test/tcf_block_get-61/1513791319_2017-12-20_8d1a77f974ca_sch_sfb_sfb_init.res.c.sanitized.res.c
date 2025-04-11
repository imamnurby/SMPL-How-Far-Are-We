static int sfb_init(struct Qdisc *sch, struct nlattr *opt, struct netlink_ext_ack *extack)
{
  struct sfb_sched_data *q = qdisc_priv(sch);
  int err;
  err = tcf_block_get(&q->block, &q->filter_list, sch, extack);
  if (err)
    return err;
  q->qdisc = &noop_qdisc;
  return sfb_change(sch, opt, extack);
}