static int atm_tc_init(struct Qdisc *sch, struct nlattr *opt,
		       struct netlink_ext_ack *extack)
{
	struct atm_qdisc_data *p = qdisc_priv(sch);
	int err;
	pr_debug("atm_tc_init(sch %p,[qdisc %p],opt %p)\n", sch, p, opt);
	INIT_LIST_HEAD(&p->flows);
	INIT_LIST_HEAD(&p->link.list);
	list_add(&p->link.list, &p->flows);
	p->link.q = qdisc_create_dflt(sch->dev_queue,
				      &pfifo_qdisc_ops, sch->handle);
	if (!p->link.q)
		p->link.q = &noop_qdisc;
	pr_debug("atm_tc_init: link (%p) qdisc %p\n", &p->link, p->link.q);
	err = tcf_block_get(&p->link.block, &p->link.filter_list, sch);
	if (err)
		return err;
	p->link.vcc = NULL;
	p->link.sock = NULL;
	p->link.common.classid = sch->handle;
	p->link.ref = 1;
	tasklet_init(&p->task, sch_atm_dequeue, (unsigned long)sch);
	return 0;
}