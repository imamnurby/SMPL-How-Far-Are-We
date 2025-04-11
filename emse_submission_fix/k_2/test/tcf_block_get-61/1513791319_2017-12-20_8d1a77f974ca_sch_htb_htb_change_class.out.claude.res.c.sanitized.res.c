static int htb_change_class(struct Qdisc *sch, u32 classid, u32 parentid, struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
  int err = -EINVAL;
  struct htb_sched *q = qdisc_priv(sch);
  struct htb_class *cl = (struct htb_class *)*arg, *parent;
  struct nlattr *opt = tca[TCA_OPTIONS];
  struct nlattr *tb[TCA_HTB_MAX + 1];
  struct tc_htb_opt *hopt;
  u64 rate64, ceil64;

  if (!opt)
    goto failure;
  err = nla_parse_nested(tb, TCA_HTB_MAX, opt, htb_policy, NULL);
  if (err < 0)
    goto failure;
  err = -EINVAL;
  if (tb[TCA_HTB_PARMS] == NULL)
    goto failure;
  parent = parentid == TC_H_ROOT ? NULL : htb_find(parentid, sch);
  hopt = nla_data(tb[TCA_HTB_PARMS]);
  if (!hopt->rate.rate || !hopt->ceil.rate)
    goto failure;

  if (hopt->rate.linklayer == TC_LINKLAYER_UNAWARE)
    qdisc_put_rtab(qdisc_get_rtab(&hopt->rate, tb[TCA_HTB_RTAB], NULL));
  if (hopt->ceil.linklayer == TC_LINKLAYER_UNAWARE)
    qdisc_put_rtab(qdisc_get_rtab(&hopt->ceil, tb[TCA_HTB_CTAB], NULL));
  if (!cl)
    {
      struct Qdisc *new_q;
      int prio;
      struct
      {
        struct nlattr nla;
        struct gnet_estimator opt;
      } est = {
          .nla =
              {
                  .nla_len = nla_attr_size(sizeof(est.opt)),
                  .nla_type = TCA_RATE,
              },
          .opt =
              {

                  .interval = 2,
                  .ewma_log = 2,
              },
      };

      if (!classid || TC_H_MAJ(classid ^ sch->handle) || htb_find(classid, sch))
        goto failure;

      if (parent && parent->parent && parent->parent->level < 2)
        {
          pr_err("htb: tree is too deep\n");
          goto failure;
        }
      err = -ENOBUFS;
      cl = kzalloc(sizeof(*cl), GFP_KERNEL);
      if (!cl)
        goto failure;
      err = tcf_block_get(&cl->block, &cl->filter_list, sch, extack);
      if (err)
        {
          kfree(cl);
          goto failure;
        }
      if (htb_rate_est || tca[TCA_RATE])
        {
          err = gen_new_estimator(&cl->bstats, NULL, &cl->rate_est, NULL, qdisc_root_sleeping_running(sch), tca[TCA_RATE] ?: &est.nla);
          if (err)
            {
              tcf_block_put(cl->block);
              kfree(cl);
              goto failure;
            }
        }
      cl->children = 0;
      INIT_LIST_HEAD(&cl->un.leaf.drop_list);
      RB_CLEAR_NODE(&cl->pq_node);
      for (prio = 0; prio < TC_HTB_NUMPRIO; prio++)
        RB_CLEAR_NODE(&cl->node[prio]);

      new_q = qdisc_create_dflt(sch->dev_queue, &pfifo_qdisc_ops, classid);
      sch_tree_lock(sch);
      if (parent && !parent->level)
        {
          unsigned int qlen = parent->un.leaf.q->q.qlen;
          unsigned int backlog = parent->un.leaf.q->qstats.backlog;

          qdisc_reset(parent->un.leaf.q);
          qdisc_tree_reduce_backlog(parent->un.leaf.q, qlen, backlog);
          qdisc_destroy(parent->un.leaf.q);
          if (parent->prio_activity)
            htb_deactivate(q, parent);

          if (parent->cmode != HTB_CAN_SEND)
            {
              htb_safe_rb_erase(&parent->pq_node, &q->hlevel[0].wait_pq);
              parent->cmode = HTB_CAN_SEND;
            }
          parent->level = (parent->parent ? parent->parent->level : TC_HTB_MAXDEPTH) - 1;
          memset(&parent->un.inner, 0, sizeof(parent->un.inner));
        }

      cl->un.leaf.q = new_q ? new_q : &noop_qdisc;
      cl->common.classid = classid;
      cl->parent = parent;

      cl->tokens = PSCHED_TICKS2NS(hopt->buffer);
      cl->ctokens = PSCHED_TICKS2NS(hopt->cbuffer);
      cl->mbuffer = 60ULL * NSEC_PER_SEC;
      cl->t_c = ktime_get_ns();
      cl->cmode = HTB_CAN_SEND;

      qdisc_class_hash_insert(&q->clhash, &cl->common);
      if (parent)
        parent->children++;
      if (cl->un.leaf.q != &noop_qdisc)
        qdisc_hash_add(cl->un.leaf.q, true);
    }
  else
    {
      if (tca[TCA_RATE])
        {
          err = gen_replace_estimator(&cl->bstats, NULL, &cl->rate_est, NULL, qdisc_root_sleeping_running(sch), tca[TCA_RATE]);
          if (err)
            return err;
        }
      sch_tree_lock(sch);
    }
  rate64 = tb[TCA_HTB_RATE64] ? nla_get_u64(tb[TCA_HTB_RATE64]) : 0;
  ceil64 = tb[TCA_HTB_CEIL64] ? nla_get_u64(tb[TCA_HTB_CEIL64]) : 0;
  psched_ratecfg_precompute(&cl->rate, &hopt->rate, rate64);
  psched_ratecfg_precompute(&cl->ceil, &hopt->ceil, ceil64);

  if (!cl->level)
    {
      u64 quantum = cl->rate.rate_bytes_ps;
      do_div(quantum, q->rate2quantum);
      cl->quantum = min_t(u64, quantum, INT_MAX);
      if (!hopt->quantum && cl->quantum < 1000)
        {
          pr_warn("HTB: quantum of class %X is small. Consider r2q change.\n", cl->common.classid);
          cl->quantum = 1000;
        }
      if (!hopt->quantum && cl->quantum > 200000)
        {
          pr_warn("HTB: quantum of class %X is big. Consider r2q change.\n", cl->common.classid);
          cl->quantum = 200000;
        }
      if (hopt->quantum)
        cl->quantum = hopt->quantum;
      if ((cl->prio = hopt->prio) >= TC_HTB_NUMPRIO)
        cl->prio = TC_HTB_NUMPRIO - 1;
    }
  cl->buffer = PSCHED_TICKS2NS(hopt->buffer);
  cl->cbuffer = PSCHED_TICKS2NS(hopt->cbuffer);
  sch_tree_unlock(sch);
  qdisc_class_hash_grow(sch, &q->clhash);
  *arg = (unsigned long)cl;
  return 0;
failure:
  return err;
}