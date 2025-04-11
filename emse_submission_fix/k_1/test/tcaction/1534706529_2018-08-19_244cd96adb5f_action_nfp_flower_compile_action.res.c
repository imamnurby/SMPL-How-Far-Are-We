int nfp_flower_compile_action(struct nfp_app *app,
			      struct tc_cls_flower_offload *flow,
			      struct net_device *netdev,
			      struct nfp_fl_payload *nfp_flow)
{
	int act_len, act_cnt, err, tun_out_cnt, out_cnt;
	enum nfp_flower_tun_type tun_type;
	const struct tc_action *a;
	u32 csum_updated = 0;
	int i;
	memset(nfp_flow->action_data, 0, NFP_FL_MAX_A_SIZ);
	nfp_flow->meta.act_len = 0;
	tun_type = NFP_FL_TUNNEL_NONE;
	act_len = 0;
	act_cnt = 0;
	tun_out_cnt = 0;
	out_cnt = 0;
	tcf_exts_for_each_action(i, a, flow->exts) {
		err = nfp_flower_loop_action(app, a, flow, nfp_flow, &act_len,
					     netdev, &tun_type, &tun_out_cnt,
					     &out_cnt, &csum_updated);
		if (err)
			return err;
		act_cnt++;
	}
	/* We optimise when the action list is small, this can unfortunately
	 * not happen once we have more than one action in the action list.
	 */
	if (act_cnt > 1)
		nfp_flow->meta.shortcut = cpu_to_be32(NFP_FL_SC_ACT_NULL);
	nfp_flow->meta.act_len = act_len;
	return 0;
}