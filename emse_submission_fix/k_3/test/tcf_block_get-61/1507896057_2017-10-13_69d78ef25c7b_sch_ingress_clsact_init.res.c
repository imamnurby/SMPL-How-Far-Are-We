static int clsact_init(struct Qdisc *sch, struct nlattr *opt)
{
	struct clsact_sched_data *q = qdisc_priv(sch);
	struct net_device *dev = qdisc_dev(sch);
	int err;
	err = tcf_block_get(&q->ingress_block, &dev->ingress_cl_list, sch);
	if (err)
		return err;
	err = tcf_block_get(&q->egress_block, &dev->egress_cl_list, sch);
	if (err)
		return err;
	net_inc_ingress_queue();
	net_inc_egress_queue();
	sch->flags |= TCQ_F_CPUSTATS;
	return 0;
}