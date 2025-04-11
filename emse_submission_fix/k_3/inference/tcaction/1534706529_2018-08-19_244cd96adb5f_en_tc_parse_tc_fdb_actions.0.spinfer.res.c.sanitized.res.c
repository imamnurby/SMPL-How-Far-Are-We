static int parse_tc_fdb_actions(struct mlx5e_priv *priv, struct tcf_exts *exts, struct mlx5e_tc_flow_parse_attr *parse_attr, struct mlx5e_tc_flow *flow)
{
  struct mlx5_esw_flow_attr *attr = flow->esw_attr;
  struct mlx5e_rep_priv *rpriv = priv->ppriv;
  struct ip_tunnel_info *info = NULL;
  const struct tc_action *a;
  int i;
  bool encap = false;
  u32 action = 0;
  int err;
  if (!tcf_exts_has_actions(exts))
    return -EINVAL;
  attr->in_rep = rpriv->rep;
  attr->in_mdev = priv->mdev;
  tcf_exts_for_each_action(i, a, exts)
  {
    if (is_tcf_gact_shot(a))
      {
        action |= MLX5_FLOW_CONTEXT_ACTION_DROP | MLX5_FLOW_CONTEXT_ACTION_COUNT;
        continue;
      }
    if (is_tcf_pedit(a))
      {
        err = parse_tc_pedit_action(priv, a, MLX5_FLOW_NAMESPACE_FDB, parse_attr);
        if (err)
          return err;
        action |= MLX5_FLOW_CONTEXT_ACTION_MOD_HDR;
        attr->mirror_count = attr->out_count;
        continue;
      }
    if (is_tcf_csum(a))
      {
        if (csum_offload_supported(priv, action, tcf_csum_update_flags(a)))
          continue;
        return -EOPNOTSUPP;
      }
    if (is_tcf_mirred_egress_redirect(a) || is_tcf_mirred_egress_mirror(a))
      {
        struct mlx5e_priv *out_priv;
        struct net_device *out_dev;
        out_dev = tcf_mirred_dev(a);
        if (attr->out_count >= MLX5_MAX_FLOW_FWD_VPORTS)
          {
            pr_err("can't support more than %d output ports, can't offload forwarding\n", attr->out_count);
            return -EOPNOTSUPP;
          }
        if (switchdev_port_same_parent_id(priv->netdev, out_dev) || is_merged_eswitch_dev(priv, out_dev))
          {
            action |= MLX5_FLOW_CONTEXT_ACTION_FWD_DEST | MLX5_FLOW_CONTEXT_ACTION_COUNT;
            out_priv = netdev_priv(out_dev);
            rpriv = out_priv->ppriv;
            attr->out_rep[attr->out_count] = rpriv->rep;
            attr->out_mdev[attr->out_count++] = out_priv->mdev;
          }
        else if (encap)
          {
            parse_attr->mirred_ifindex = out_dev->ifindex;
            parse_attr->tun_info = *info;
            attr->parse_attr = parse_attr;
            action |= MLX5_FLOW_CONTEXT_ACTION_ENCAP | MLX5_FLOW_CONTEXT_ACTION_FWD_DEST | MLX5_FLOW_CONTEXT_ACTION_COUNT;
          }
        else
          {
            pr_err("devices %s %s not on same switch HW, can't offload forwarding\n", priv->netdev->name, out_dev->name);
            return -EINVAL;
          }
        continue;
      }
    if (is_tcf_tunnel_set(a))
      {
        info = tcf_tunnel_info(a);
        if (info)
          encap = true;
        else
          return -EOPNOTSUPP;
        attr->mirror_count = attr->out_count;
        continue;
      }
    if (is_tcf_vlan(a))
      {
        err = parse_tc_vlan_action(priv, a, attr, &action);
        if (err)
          return err;
        attr->mirror_count = attr->out_count;
        continue;
      }
    if (is_tcf_tunnel_release(a))
      {
        action |= MLX5_FLOW_CONTEXT_ACTION_DECAP;
        continue;
      }
    return -EINVAL;
  }
  attr->action = action;
  if (!actions_match_supported(priv, exts, parse_attr, flow))
    return -EOPNOTSUPP;
  if (attr->out_count > 1 && !mlx5_esw_has_fwd_fdb(priv->mdev))
    {
      netdev_warn_once(priv->netdev, "current firmware doesn't support split rule for port mirroring\n");
      return -EOPNOTSUPP;
    }
  return 0;
}