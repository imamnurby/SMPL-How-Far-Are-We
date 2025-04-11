static int fq_codel_init(struct Qdisc *sch, struct nlattr *opt,
			 struct netlink_ext_ack *extack)
{
	struct fq_codel_sched_data *q = qdisc_priv(sch);
	int i;
	int err;
	sch->limit = 10*1024;
	q->flows_cnt = 1024;
	q->memory_limit = 32 << 20; /* 32 MBytes */
	q->drop_batch_size = 64;
	q->quantum = psched_mtu(qdisc_dev(sch));
	INIT_LIST_HEAD(&q->new_flows);
	INIT_LIST_HEAD(&q->old_flows);
	codel_params_init(&q->cparams);
	codel_stats_init(&q->cstats);
	q->cparams.ecn = true;
	q->cparams.mtu = psched_mtu(qdisc_dev(sch));
	if (opt) {
		int err = fq_codel_change(sch, opt, extack);
		if (err)
			return err;
	}
	err = tcf_block_get(&q->block, &q->filter_list, sch);
	if (err)
		return err;
	if (!q->flows) {
		q->flows = kvzalloc(q->flows_cnt *
					   sizeof(struct fq_codel_flow), GFP_KERNEL);
		if (!q->flows)
			return -ENOMEM;
		q->backlogs = kvzalloc(q->flows_cnt * sizeof(u32), GFP_KERNEL);
		if (!q->backlogs)
			return -ENOMEM;
		for (i = 0; i < q->flows_cnt; i++) {
			struct fq_codel_flow *flow = q->flows + i;
			INIT_LIST_HEAD(&flow->flowchain);
			codel_vars_init(&flow->cvars);
		}
	}
	if (sch->limit >= 1)
		sch->flags |= TCQ_F_CAN_BYPASS;
	else
		sch->flags &= ~TCQ_F_CAN_BYPASS;
	return 0;
}