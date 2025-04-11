static int cxgb4_validate_flow_actions(struct net_device *dev, struct tc_cls_flower_offload *cls)
{
  const struct tc_action *a;
  bool act_redir = false;
  bool act_pedit = false;
  bool act_vlan = false;
  int i;tcf_exts_for_each_action(i, a, cls->exts)
  {
    if (is_tcf_gact_ok(a))
      {
      }
    else if (is_tcf_gact_shot(a))
      {
      }
    else if (is_tcf_mirred_egress_redirect(a))
      {
        struct adapter *adap = netdev2adap(dev);
        struct net_device *n_dev, *target_dev;
        unsigned int i;
        bool found = false;
        target_dev = tcf_mirred_dev(a);
        for_each_port(adap, i)
        {
          n_dev = adap->port[i];
          if (target_dev == n_dev)
            {
              found = true;
              break;
            }
        }

        if (!found)
          {
            netdev_err(dev, "%s: Out port invalid\n", __func__);
            return -EINVAL;
          }
        act_redir = true;
      }
    else if (is_tcf_vlan(a))
      {
        u16 proto = be16_to_cpu(tcf_vlan_push_proto(a));
        u32 vlan_action = tcf_vlan_action(a);
        switch (vlan_action)
          {
          case TCA_VLAN_ACT_POP:
            break;
          case TCA_VLAN_ACT_PUSH:
          case TCA_VLAN_ACT_MODIFY:
            if (proto != ETH_P_8021Q)
              {
                netdev_err(dev, "%s: Unsupported vlan proto\n", __func__);
                return -EOPNOTSUPP;
              }
            break;
          default:
            netdev_err(dev, "%s: Unsupported vlan action\n", __func__);
            return -EOPNOTSUPP;
          }
        act_vlan = true;
      }
    else if (is_tcf_pedit(a))
      {
        bool pedit_valid = valid_pedit_action(dev, a);
        if (!pedit_valid)
          return -EOPNOTSUPP;
        act_pedit = true;
      }
    else
      {
        netdev_err(dev, "%s: Unsupported action\n", __func__);
        return -EOPNOTSUPP;
      }
  }
  if ((act_pedit || act_vlan) && !act_redir)
    {
      netdev_err(dev, "%s: pedit/vlan rewrite invalid without egress redirect\n", __func__);
      return -EINVAL;
    }
  return 0;
}