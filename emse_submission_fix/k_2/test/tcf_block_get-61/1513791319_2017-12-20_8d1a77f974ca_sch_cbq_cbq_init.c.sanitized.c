static int cbq_init(struct Qdisc *sch, struct nlattr *opt, struct netlink_ext_ack *extack)
{
  struct cbq_sched_data *q = qdisc_priv(sch);
  struct nlattr *tb[TCA_CBQ_MAX + 1];
  struct tc_ratespec *r;
  int err;
  qdisc_watchdog_init(&q->watchdog, sch);
  hrtimer_init(&q->delay_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED);
  q->delay_timer.function = cbq_undelay;
  if (!opt)
    return -EINVAL;
  err = nla_parse_nested(tb, TCA_CBQ_MAX, opt, cbq_policy, NULL);
  if (err < 0)
    return err;
  if (!tb[TCA_CBQ_RTAB] || !tb[TCA_CBQ_RATE])
    return -EINVAL;
  r = nla_data(tb[TCA_CBQ_RATE]);
  q->link.R_tab = qdisc_get_rtab(r, tb[TCA_CBQ_RTAB], extack);
  if (!q->link.R_tab)
    return -EINVAL;
  err = tcf_block_get(&q->link.block, &q->link.filter_list, sch);
  if (err)
    goto put_rtab;
  err = qdisc_class_hash_init(&q->clhash);
  if (err < 0)
    goto put_block;
  q->link.sibling = &q->link;
  q->link.common.classid = sch->handle;
  q->link.qdisc = sch;
  q->link.q = qdisc_create_dflt(sch->dev_queue, &pfifo_qdisc_ops, sch->handle);
  if (!q->link.q)
    q->link.q = &noop_qdisc;
  else
    qdisc_hash_add(q->link.q, true);
  q->link.priority = TC_CBQ_MAXPRIO - 1;
  q->link.priority2 = TC_CBQ_MAXPRIO - 1;
  q->link.cpriority = TC_CBQ_MAXPRIO - 1;
  q->link.allot = psched_mtu(qdisc_dev(sch));
  q->link.quantum = q->link.allot;
  q->link.weight = q->link.R_tab->rate.rate;
  q->link.ewma_log = TC_CBQ_DEF_EWMA;
  q->link.avpkt = q->link.allot / 2;
  q->link.minidle = -0x7FFFFFFF;
  q->toplevel = TC_CBQ_MAXLEVEL;
  q->now = psched_get_time();
  cbq_link_class(&q->link);
  if (tb[TCA_CBQ_LSSOPT])
    cbq_set_lss(&q->link, nla_data(tb[TCA_CBQ_LSSOPT]));
  cbq_addprio(q, &q->link);
  return 0;
put_block:
  tcf_block_put(q->link.block);
put_rtab:
  qdisc_put_rtab(q->link.R_tab);
  return err;
}