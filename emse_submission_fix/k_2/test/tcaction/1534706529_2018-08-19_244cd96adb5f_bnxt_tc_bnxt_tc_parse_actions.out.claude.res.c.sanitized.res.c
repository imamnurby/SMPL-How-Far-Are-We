static int bnxt_tc_parse_actions(struct bnxt *bp, struct bnxt_tc_actions *actions, struct tcf_exts *tc_exts)
{
  const struct tc_action *tc_act;
  LIST_HEAD(tc_actions);
  int rc;
  if (!tcf_exts_has_actions(tc_exts))
    {
      netdev_info(bp->dev, "no actions");
      return -EINVAL;
    }
  tcf_exts_to_list(tc_exts, &tc_actions);
  list_for_each_entry(tc_act, &tc_actions, list)
  {

    if (is_tcf_gact_shot(tc_act))
      {
        actions->flags |= BNXT_TC_ACTION_FLAG_DROP;
        return 0;
      }

    if (is_tcf_mirred_egress_redirect(tc_act))
      {
        rc = bnxt_tc_parse_redir(bp, actions, tc_act);
        if (rc)
          return rc;
        continue;
      }

    if (is_tcf_vlan(tc_act))
      {
        bnxt_tc_parse_vlan(bp, actions, tc_act);
        continue;
      }

    if (is_tcf_tunnel_set(tc_act))
      {
        rc = bnxt_tc_parse_tunnel_set(bp, actions, tc_act);
        if (rc)
          return rc;
        continue;
      }

    if (is_tcf_tunnel_release(tc_act))
      {
        actions->flags |= BNXT_TC_ACTION_FLAG_TUNNEL_DECAP;
        continue;
      }
  }
  if (actions->flags & BNXT_TC_ACTION_FLAG_FWD)
    {
      if (actions->flags & BNXT_TC_ACTION_FLAG_TUNNEL_ENCAP)
        {

          actions->dst_fid = bp->pf.fw_fid;
        }
      else
        {

          actions->dst_fid = bnxt_flow_get_dst_fid(bp, actions->dst_dev);
          if (actions->dst_fid == BNXT_FID_INVALID)
            return -EINVAL;
        }
    }
  return 0;
}