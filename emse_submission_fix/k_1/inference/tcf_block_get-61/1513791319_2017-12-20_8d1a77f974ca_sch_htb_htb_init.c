static int htb_init(struct Qdisc *sch, struct nlattr *opt,
		    struct netlink_ext_ack *extack)
{
	struct htb_sched *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_HTB_MAX + 1];
	struct tc_htb_glob *gopt;
	int err;
	int i;
	qdisc_watchdog_init(&q->watchdog, sch);
	INIT_WORK(&q->work, htb_work_func);
	if (!opt)
		return -EINVAL;
	err = tcf_block_get(&q->block, &q->filter_list, sch);
	if (err)
		return err;
	err = nla_parse_nested(tb, TCA_HTB_MAX, opt, htb_policy, NULL);
	if (err < 0)
		return err;
	if (!tb[TCA_HTB_INIT])
		return -EINVAL;
	gopt = nla_data(tb[TCA_HTB_INIT]);
	if (gopt->version != HTB_VER >> 16)
		return -EINVAL;
	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0)
		return err;
	for (i = 0; i < TC_HTB_NUMPRIO; i++)
		INIT_LIST_HEAD(q->drops + i);
	qdisc_skb_head_init(&q->direct_queue);
	if (tb[TCA_HTB_DIRECT_QLEN])
		q->direct_qlen = nla_get_u32(tb[TCA_HTB_DIRECT_QLEN]);
	else
		q->direct_qlen = qdisc_dev(sch)->tx_queue_len;
	if ((q->rate2quantum = gopt->rate2quantum) < 1)
		q->rate2quantum = 1;
	q->defcls = gopt->defcls;
	return 0;
}