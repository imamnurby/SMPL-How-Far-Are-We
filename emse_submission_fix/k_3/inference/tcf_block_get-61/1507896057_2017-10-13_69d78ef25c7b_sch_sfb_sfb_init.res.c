static int sfb_init(struct Qdisc *sch, struct nlattr *opt)
{
	struct sfb_sched_data *q = qdisc_priv(sch);
	int err;
	err = tcf_block_get(&q->block, &q->filter_list, sch);
	if (err)
		return err;
	q->qdisc = &noop_qdisc;
	return sfb_change(sch, opt);
}