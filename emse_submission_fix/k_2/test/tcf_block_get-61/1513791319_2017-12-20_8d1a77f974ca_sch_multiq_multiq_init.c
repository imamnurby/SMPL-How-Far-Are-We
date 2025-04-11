static int multiq_init(struct Qdisc *sch, struct nlattr *opt,
		       struct netlink_ext_ack *extack)
{
	struct multiq_sched_data *q = qdisc_priv(sch);
	int i, err;
	q->queues = NULL;
	if (!opt)
		return -EINVAL;
	err = tcf_block_get(&q->block, &q->filter_list, sch);
	if (err)
		return err;
	q->max_bands = qdisc_dev(sch)->num_tx_queues;
	q->queues = kcalloc(q->max_bands, sizeof(struct Qdisc *), GFP_KERNEL);
	if (!q->queues)
		return -ENOBUFS;
	for (i = 0; i < q->max_bands; i++)
		q->queues[i] = &noop_qdisc;
	return multiq_tune(sch, opt, extack);
}