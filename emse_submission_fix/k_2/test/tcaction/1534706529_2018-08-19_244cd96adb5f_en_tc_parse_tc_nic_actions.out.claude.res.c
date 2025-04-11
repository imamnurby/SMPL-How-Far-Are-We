static int parse_tc_nic_actions(struct mlx5e_priv *priv, struct tcf_exts *exts, struct mlx5e_tc_flow_parse_attr *parse_attr, struct mlx5e_tc_flow *flow)
{
  struct mlx5_nic_flow_attr *attr = flow->nic_attr;
  const struct tc_action *a;
  LIST_HEAD(actions);
  u32 action = 0;
  int err;
  if (!tcf_exts_has_actions(exts))
    return -EINVAL;
  attr->flow_tag = MLX5_FS_DEFAULT_FLOW_TAG;
  tcf_exts_to_list(exts, &actions);
  list_for_each_entry(a, &actions, list)
  {
    if (is_tcf_gact_shot(a))
      {
        action |= MLX5_FLOW_CONTEXT_ACTION_DROP;
        if (MLX5_CAP_FLOWTABLE(priv->mdev, flow_table_properties_nic_receive.flow_counter))
          action |= MLX5_FLOW_CONTEXT_ACTION_COUNT;
        continue;
      }
    if (is_tcf_pedit(a))
      {
        err = parse_tc_pedit_action(priv, a, MLX5_FLOW_NAMESPACE_KERNEL, parse_attr);
        if (err)
          return err;
        action |= MLX5_FLOW_CONTEXT_ACTION_MOD_HDR | MLX5_FLOW_CONTEXT_ACTION_FWD_DEST;
        continue;
      }
    if (is_tcf_csum(a))
      {
        if (csum_offload_supported(priv, action, tcf_csum_update_flags(a)))
          continue;
        return -EOPNOTSUPP;
      }
    if (is_tcf_mirred_egress_redirect(a))
      {
        struct net_device *peer_dev = tcf_mirred_dev(a);
        if (priv->netdev->netdev_ops == peer_dev->netdev_ops && same_hw_devs(priv, netdev_priv(peer_dev)))
          {
            parse_attr->mirred_ifindex = peer_dev->ifindex;
            flow->flags |= MLX5E_TC_FLOW_HAIRPIN;
            action |= MLX5_FLOW_CONTEXT_ACTION_FWD_DEST | MLX5_FLOW_CONTEXT_ACTION_COUNT;
          }
        else
          {
            netdev_warn(priv->netdev, "device %s not on same HW, can't offload\n", peer_dev->name);
            return -EINVAL;
          }
        continue;
      }
    if (is_tcf_skbedit_mark(a))
      {
        u32 mark = tcf_skbedit_mark(a);
        if (mark & ~MLX5E_TC_FLOW_ID_MASK)
          {
            netdev_warn(priv->netdev, "Bad flow mark - only 16 bit is supported: 0x%x\n", mark);
            return -EINVAL;
          }
        attr->flow_tag = mark;
        action |= MLX5_FLOW_CONTEXT_ACTION_FWD_DEST;
        continue;
      }
    return -EINVAL;
  }
  attr->action = action;
  if (!actions_match_supported(priv, exts, parse_attr, flow))
    return -EOPNOTSUPP;
  return 0;
}