static int
hfsc_init_qdisc(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
{
	struct hfsc_sched *q = qdisc_priv(sch);
	struct tc_hfsc_qopt *qopt;
	int err;
	qdisc_watchdog_init(&q->watchdog, sch);
	if (!opt || nla_len(opt) < sizeof(*qopt))
		return -EINVAL;
	qopt = nla_data(opt);
	q->defcls = qopt->defcls;
	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0)
		return err;
	q->eligible = RB_ROOT;
	err = tcf_block_get(&q->root.block, &q->root.filter_list, sch, extack);
	if (err)
		return err;
	q->root.cl_common.classid = sch->handle;
	q->root.sched   = q;
	q->root.qdisc = qdisc_create_dflt(sch->dev_queue, &pfifo_qdisc_ops,
					  sch->handle);
	if (q->root.qdisc == NULL)
		q->root.qdisc = &noop_qdisc;
	else
		qdisc_hash_add(q->root.qdisc, true);
	INIT_LIST_HEAD(&q->root.children);
	q->root.vt_tree = RB_ROOT;
	q->root.cf_tree = RB_ROOT;
	qdisc_class_hash_insert(&q->clhash, &q->root.cl_common);
	qdisc_class_hash_grow(sch, &q->clhash);
	return 0;
}