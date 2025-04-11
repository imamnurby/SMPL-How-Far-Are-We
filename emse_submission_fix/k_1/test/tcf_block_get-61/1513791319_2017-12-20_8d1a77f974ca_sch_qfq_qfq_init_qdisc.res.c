static int qfq_init_qdisc(struct Qdisc *sch, struct nlattr *opt,
			  struct netlink_ext_ack *extack)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_group *grp;
	int i, j, err;
	u32 max_cl_shift, maxbudg_shift, max_classes;
	err = tcf_block_get(&q->block, &q->filter_list, sch, extack);
	if (err)
		return err;
	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0)
		return err;
	if (qdisc_dev(sch)->tx_queue_len + 1 > QFQ_MAX_AGG_CLASSES)
		max_classes = QFQ_MAX_AGG_CLASSES;
	else
		max_classes = qdisc_dev(sch)->tx_queue_len + 1;
	/* max_cl_shift = floor(log_2(max_classes)) */
	max_cl_shift = __fls(max_classes);
	q->max_agg_classes = 1<<max_cl_shift;
	/* maxbudg_shift = log2(max_len * max_classes_per_agg) */
	maxbudg_shift = QFQ_MTU_SHIFT + max_cl_shift;
	q->min_slot_shift = FRAC_BITS + maxbudg_shift - QFQ_MAX_INDEX;
	for (i = 0; i <= QFQ_MAX_INDEX; i++) {
		grp = &q->groups[i];
		grp->index = i;
		grp->slot_shift = q->min_slot_shift + i;
		for (j = 0; j < QFQ_MAX_SLOTS; j++)
			INIT_HLIST_HEAD(&grp->slots[j]);
	}
	INIT_HLIST_HEAD(&q->nonfull_aggs);
	return 0;
}