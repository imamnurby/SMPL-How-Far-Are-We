static int prio_init(struct Qdisc *sch, struct nlattr *opt,
		     struct netlink_ext_ack *extack)
{
	struct prio_sched_data *q = qdisc_priv(sch);
	int err;
	if (!opt)
		return -EINVAL;
	err = tcf_block_get(&q->block, &q->filter_list, sch);
	if (err)
		return err;
	return prio_tune(sch, opt, extack);
}