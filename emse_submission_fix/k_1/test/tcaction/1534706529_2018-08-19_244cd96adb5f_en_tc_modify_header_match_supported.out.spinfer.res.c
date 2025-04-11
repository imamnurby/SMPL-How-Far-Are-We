static bool modify_header_match_supported(struct mlx5_flow_spec *spec, struct tcf_exts *exts)
{
  const struct tc_action *a;
  bool modify_ip_header;
  int i;
  u8 htype, ip_proto;
  void *headers_v;
  u16 ethertype;
  int nkeys, i;
  headers_v = MLX5_ADDR_OF(fte_match_param, spec->match_value, outer_headers);
  ethertype = MLX5_GET(fte_match_set_lyr_2_4, headers_v, ethertype);

  if (ethertype != ETH_P_IP && ethertype != ETH_P_IPV6)
    goto out_ok;
  modify_ip_header = false;
  tcf_exts_for_each_action(i, a, exts)
  {
    if (!is_tcf_pedit(a))
      continue;
    nkeys = tcf_pedit_nkeys(a);
    for (i = 0; i < nkeys; i++)
      {
        htype = tcf_pedit_htype(a, i);
        if (htype == TCA_PEDIT_KEY_EX_HDR_TYPE_IP4 || htype == TCA_PEDIT_KEY_EX_HDR_TYPE_IP6)
          {
            modify_ip_header = true;
            break;
          }
      }
  }
  ip_proto = MLX5_GET(fte_match_set_lyr_2_4, headers_v, ip_protocol);
  if (modify_ip_header && ip_proto != IPPROTO_TCP && ip_proto != IPPROTO_UDP && ip_proto != IPPROTO_ICMP)
    {
      pr_info("can't offload re-write of ip proto %d\n", ip_proto);
      return false;
    }
out_ok:
  return true;
}