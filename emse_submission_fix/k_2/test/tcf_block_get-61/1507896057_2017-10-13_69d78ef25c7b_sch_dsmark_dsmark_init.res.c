static int dsmark_init(struct Qdisc *sch, struct nlattr *opt)
{
	struct dsmark_qdisc_data *p = qdisc_priv(sch);
	struct nlattr *tb[TCA_DSMARK_MAX + 1];
	int err = -EINVAL;
	u32 default_index = NO_DEFAULT_INDEX;
	u16 indices;
	int i;
	pr_debug("%s(sch %p,[qdisc %p],opt %p)\n", __func__, sch, p, opt);
	if (!opt)
		goto errout;
	err = tcf_block_get(&p->block, &p->filter_list, sch);
	if (err)
		return err;
	err = nla_parse_nested(tb, TCA_DSMARK_MAX, opt, dsmark_policy, NULL);
	if (err < 0)
		goto errout;
	err = -EINVAL;
	indices = nla_get_u16(tb[TCA_DSMARK_INDICES]);
	if (hweight32(indices) != 1)
		goto errout;
	if (tb[TCA_DSMARK_DEFAULT_INDEX])
		default_index = nla_get_u16(tb[TCA_DSMARK_DEFAULT_INDEX]);
	if (indices <= DSMARK_EMBEDDED_SZ)
		p->mv = p->embedded;
	else
		p->mv = kmalloc_array(indices, sizeof(*p->mv), GFP_KERNEL);
	if (!p->mv) {
		err = -ENOMEM;
		goto errout;
	}
	for (i = 0; i < indices; i++) {
		p->mv[i].mask = 0xff;
		p->mv[i].value = 0;
	}
	p->indices = indices;
	p->default_index = default_index;
	p->set_tc_index = nla_get_flag(tb[TCA_DSMARK_SET_TC_INDEX]);
	p->q = qdisc_create_dflt(sch->dev_queue, &pfifo_qdisc_ops, sch->handle);
	if (p->q == NULL)
		p->q = &noop_qdisc;
	else
		qdisc_hash_add(p->q, true);
	pr_debug("%s: qdisc %p\n", __func__, p->q);
	err = 0;
errout:
	return err;
}