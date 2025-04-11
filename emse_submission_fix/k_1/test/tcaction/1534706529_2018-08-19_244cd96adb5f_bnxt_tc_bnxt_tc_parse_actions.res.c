static int bnxt_tc_parse_actions(struct bnxt *bp,
				 struct bnxt_tc_actions *actions,
				 struct tcf_exts *tc_exts)
{
	const struct tc_action *tc_act;
	int i;
	int rc;
	if (!tcf_exts_has_actions(tc_exts)) {
		netdev_info(bp->dev, "no actions");
		return -EINVAL;
	}
	tcf_exts_for_each_action(i, tc_act, tc_exts) {
		/* Drop action */
		if (is_tcf_gact_shot(tc_act)) {
			actions->flags |= BNXT_TC_ACTION_FLAG_DROP;
			return 0; /* don't bother with other actions */
		}
		/* Redirect action */
		if (is_tcf_mirred_egress_redirect(tc_act)) {
			rc = bnxt_tc_parse_redir(bp, actions, tc_act);
			if (rc)
				return rc;
			continue;
		}
		/* Push/pop VLAN */
		if (is_tcf_vlan(tc_act)) {
			bnxt_tc_parse_vlan(bp, actions, tc_act);
			continue;
		}
		/* Tunnel encap */
		if (is_tcf_tunnel_set(tc_act)) {
			rc = bnxt_tc_parse_tunnel_set(bp, actions, tc_act);
			if (rc)
				return rc;
			continue;
		}
		/* Tunnel decap */
		if (is_tcf_tunnel_release(tc_act)) {
			actions->flags |= BNXT_TC_ACTION_FLAG_TUNNEL_DECAP;
			continue;
		}
	}
	if (actions->flags & BNXT_TC_ACTION_FLAG_FWD) {
		if (actions->flags & BNXT_TC_ACTION_FLAG_TUNNEL_ENCAP) {
			/* dst_fid is PF's fid */
			actions->dst_fid = bp->pf.fw_fid;
		} else {
			/* find the FID from dst_dev */
			actions->dst_fid =
				bnxt_flow_get_dst_fid(bp, actions->dst_dev);
			if (actions->dst_fid == BNXT_FID_INVALID)
				return -EINVAL;
		}
	}
	return 0;
}