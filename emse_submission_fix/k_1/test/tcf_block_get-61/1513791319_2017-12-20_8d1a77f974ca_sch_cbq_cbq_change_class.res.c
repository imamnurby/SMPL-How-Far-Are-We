static int
cbq_change_class(struct Qdisc *sch, u32 classid, u32 parentid, struct nlattr **tca,
		 unsigned long *arg, struct netlink_ext_ack *extack)
{
	int err;
	struct cbq_sched_data *q = qdisc_priv(sch);
	struct cbq_class *cl = (struct cbq_class *)*arg;
	struct nlattr *opt = tca[TCA_OPTIONS];
	struct nlattr *tb[TCA_CBQ_MAX + 1];
	struct cbq_class *parent;
	struct qdisc_rate_table *rtab = NULL;
	if (!opt)
		return -EINVAL;
	err = nla_parse_nested(tb, TCA_CBQ_MAX, opt, cbq_policy, NULL);
	if (err < 0)
		return err;
	if (tb[TCA_CBQ_OVL_STRATEGY] || tb[TCA_CBQ_POLICE])
		return -EOPNOTSUPP;
	if (cl) {
		/* Check parent */
		if (parentid) {
			if (cl->tparent &&
			    cl->tparent->common.classid != parentid)
				return -EINVAL;
			if (!cl->tparent && parentid != TC_H_ROOT)
				return -EINVAL;
		}
		if (tb[TCA_CBQ_RATE]) {
			rtab = qdisc_get_rtab(nla_data(tb[TCA_CBQ_RATE]),
					      tb[TCA_CBQ_RTAB], extack);
			if (rtab == NULL)
				return -EINVAL;
		}
		if (tca[TCA_RATE]) {
			err = gen_replace_estimator(&cl->bstats, NULL,
						    &cl->rate_est,
						    NULL,
						    qdisc_root_sleeping_running(sch),
						    tca[TCA_RATE]);
			if (err) {
				qdisc_put_rtab(rtab);
				return err;
			}
		}
		/* Change class parameters */
		sch_tree_lock(sch);
		if (cl->next_alive != NULL)
			cbq_deactivate_class(cl);
		if (rtab) {
			qdisc_put_rtab(cl->R_tab);
			cl->R_tab = rtab;
		}
		if (tb[TCA_CBQ_LSSOPT])
			cbq_set_lss(cl, nla_data(tb[TCA_CBQ_LSSOPT]));
		if (tb[TCA_CBQ_WRROPT]) {
			cbq_rmprio(q, cl);
			cbq_set_wrr(cl, nla_data(tb[TCA_CBQ_WRROPT]));
		}
		if (tb[TCA_CBQ_FOPT])
			cbq_set_fopt(cl, nla_data(tb[TCA_CBQ_FOPT]));
		if (cl->q->q.qlen)
			cbq_activate_class(cl);
		sch_tree_unlock(sch);
		return 0;
	}
	if (parentid == TC_H_ROOT)
		return -EINVAL;
	if (!tb[TCA_CBQ_WRROPT] || !tb[TCA_CBQ_RATE] || !tb[TCA_CBQ_LSSOPT])
		return -EINVAL;
	rtab = qdisc_get_rtab(nla_data(tb[TCA_CBQ_RATE]), tb[TCA_CBQ_RTAB],
			      extack);
	if (rtab == NULL)
		return -EINVAL;
	if (classid) {
		err = -EINVAL;
		if (TC_H_MAJ(classid ^ sch->handle) ||
		    cbq_class_lookup(q, classid))
			goto failure;
	} else {
		int i;
		classid = TC_H_MAKE(sch->handle, 0x8000);
		for (i = 0; i < 0x8000; i++) {
			if (++q->hgenerator >= 0x8000)
				q->hgenerator = 1;
			if (cbq_class_lookup(q, classid|q->hgenerator) == NULL)
				break;
		}
		err = -ENOSR;
		if (i >= 0x8000)
			goto failure;
		classid = classid|q->hgenerator;
	}
	parent = &q->link;
	if (parentid) {
		parent = cbq_class_lookup(q, parentid);
		err = -EINVAL;
		if (!parent)
			goto failure;
	}
	err = -ENOBUFS;
	cl = kzalloc(sizeof(*cl), GFP_KERNEL);
	if (cl == NULL)
		goto failure;
	err = tcf_block_get(&cl->block, &cl->filter_list, sch, extack);
	if (err) {
		kfree(cl);
		return err;
	}
	if (tca[TCA_RATE]) {
		err = gen_new_estimator(&cl->bstats, NULL, &cl->rate_est,
					NULL,
					qdisc_root_sleeping_running(sch),
					tca[TCA_RATE]);
		if (err) {
			tcf_block_put(cl->block);
			kfree(cl);
			goto failure;
		}
	}
	cl->R_tab = rtab;
	rtab = NULL;
	cl->q = qdisc_create_dflt(sch->dev_queue, &pfifo_qdisc_ops, classid);
	if (!cl->q)
		cl->q = &noop_qdisc;
	else
		qdisc_hash_add(cl->q, true);
	cl->common.classid = classid;
	cl->tparent = parent;
	cl->qdisc = sch;
	cl->allot = parent->allot;
	cl->quantum = cl->allot;
	cl->weight = cl->R_tab->rate.rate;
	sch_tree_lock(sch);
	cbq_link_class(cl);
	cl->borrow = cl->tparent;
	if (cl->tparent != &q->link)
		cl->share = cl->tparent;
	cbq_adjust_levels(parent);
	cl->minidle = -0x7FFFFFFF;
	cbq_set_lss(cl, nla_data(tb[TCA_CBQ_LSSOPT]));
	cbq_set_wrr(cl, nla_data(tb[TCA_CBQ_WRROPT]));
	if (cl->ewma_log == 0)
		cl->ewma_log = q->link.ewma_log;
	if (cl->maxidle == 0)
		cl->maxidle = q->link.maxidle;
	if (cl->avpkt == 0)
		cl->avpkt = q->link.avpkt;
	if (tb[TCA_CBQ_FOPT])
		cbq_set_fopt(cl, nla_data(tb[TCA_CBQ_FOPT]));
	sch_tree_unlock(sch);
	qdisc_class_hash_grow(sch, &q->clhash);
	*arg = (unsigned long)cl;
	return 0;
failure:
	qdisc_put_rtab(rtab);
	return err;
}