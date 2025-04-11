static int atm_tc_change(struct Qdisc *sch, u32 classid, u32 parent, struct nlattr **tca, unsigned long *arg)
{
  struct atm_qdisc_data *p = qdisc_priv(sch);
  struct atm_flow_data *flow = (struct atm_flow_data *)*arg;
  struct atm_flow_data *excess = NULL;
  struct nlattr *opt = tca[TCA_OPTIONS];
  struct nlattr *tb[TCA_ATM_MAX + 1];
  struct socket *sock;
  int fd, error, hdr_len;
  void *hdr;
  pr_debug("atm_tc_change(sch %p,[qdisc %p],classid %x,parent %x,"
           "flow %p,opt %p)\n",
           sch,
           p,
           classid,
           parent,
           flow,
           opt);

  if (parent && parent != TC_H_ROOT && parent != sch->handle)
    return -EINVAL;

  if (flow)
    return -EBUSY;
  if (opt == NULL)
    return -EINVAL;
  error = nla_parse_nested(tb, TCA_ATM_MAX, opt, atm_policy, NULL);
  if (error < 0)
    return error;
  if (!tb[TCA_ATM_FD])
    return -EINVAL;
  fd = nla_get_u32(tb[TCA_ATM_FD]);
  pr_debug("atm_tc_change: fd %d\n", fd);
  if (tb[TCA_ATM_HDR])
    {
      hdr_len = nla_len(tb[TCA_ATM_HDR]);
      hdr = nla_data(tb[TCA_ATM_HDR]);
    }
  else
    {
      hdr_len = RFC1483LLC_LEN;
      hdr = NULL;
    }
  if (!tb[TCA_ATM_EXCESS])
    excess = NULL;
  else
    {
      excess = (struct atm_flow_data *)atm_tc_find(sch, nla_get_u32(tb[TCA_ATM_EXCESS]));
      if (!excess)
        return -ENOENT;
    }
  pr_debug("atm_tc_change: type %d, payload %d, hdr_len %d\n", opt->nla_type, nla_len(opt), hdr_len);
  sock = sockfd_lookup(fd, &error);
  if (!sock)
    return error;
  pr_debug("atm_tc_change: f_count %ld\n", file_count(sock->file));
  if (sock->ops->family != PF_ATMSVC && sock->ops->family != PF_ATMPVC)
    {
      error = -EPROTOTYPE;
      goto err_out;
    }

  if (classid)
    {
      if (TC_H_MAJ(classid ^ sch->handle))
        {
          pr_debug("atm_tc_change: classid mismatch\n");
          error = -EINVAL;
          goto err_out;
        }
    }
  else
    {
      int i;
      unsigned long cl;
      for (i = 1; i < 0x8000; i++)
        {
          classid = TC_H_MAKE(sch->handle, 0x8000 | i);
          cl = atm_tc_find(sch, classid);
          if (!cl)
            break;
        }
    }
  pr_debug("atm_tc_change: new id %x\n", classid);
  flow = kzalloc(sizeof(struct atm_flow_data) + hdr_len, GFP_KERNEL);
  pr_debug("atm_tc_change: flow %p\n", flow);
  if (!flow)
    {
      error = -ENOBUFS;
      goto err_out;
    }
  error = tcf_block_get(&flow->block, &flow->filter_list);
  if (error)
    {
      kfree(flow);
      goto err_out;
    }
  flow->q = qdisc_create_dflt(sch->dev_queue, &pfifo_qdisc_ops, classid);
  if (!flow->q)
    flow->q = &noop_qdisc;
  pr_debug("atm_tc_change: qdisc %p\n", flow->q);
  flow->sock = sock;
  flow->vcc = ATM_SD(sock);
  flow->vcc->user_back = flow;
  pr_debug("atm_tc_change: vcc %p\n", flow->vcc);
  flow->old_pop = flow->vcc->pop;
  flow->parent = p;
  flow->vcc->pop = sch_atm_pop;
  flow->common.classid = classid;
  flow->ref = 1;
  flow->excess = excess;
  list_add(&flow->list, &p->link.list);
  flow->hdr_len = hdr_len;
  if (hdr)
    memcpy(flow->hdr, hdr, hdr_len);
  else
    memcpy(flow->hdr, llc_oui_ip, sizeof(llc_oui_ip));
  *arg = (unsigned long)flow;
  return 0;
err_out:
  sockfd_put(sock);
  return error;
}