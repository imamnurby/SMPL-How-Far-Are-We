static void cxgb4_process_flow_actions(struct net_device *in, struct tc_cls_flower_offload *cls, struct ch_filter_specification *fs)
{
  const struct tc_action *a;
  LIST_HEAD(actions);
  tcf_exts_to_list(cls->exts, &actions);
  list_for_each_entry(a, &actions, list)
  {
    if (is_tcf_gact_ok(a))
      {
        fs->action = FILTER_PASS;
      }
    else if (is_tcf_gact_shot(a))
      {
        fs->action = FILTER_DROP;
      }
    else if (is_tcf_mirred_egress_redirect(a))
      {
        struct net_device *out = tcf_mirred_dev(a);
        struct port_info *pi = netdev_priv(out);
        fs->action = FILTER_SWITCH;
        fs->eport = pi->port_id;
      }
    else if (is_tcf_vlan(a))
      {
        u32 vlan_action = tcf_vlan_action(a);
        u8 prio = tcf_vlan_push_prio(a);
        u16 vid = tcf_vlan_push_vid(a);
        u16 vlan_tci = (prio << VLAN_PRIO_SHIFT) | vid;
        switch (vlan_action)
          {
          case TCA_VLAN_ACT_POP:
            fs->newvlan |= VLAN_REMOVE;
            break;
          case TCA_VLAN_ACT_PUSH:
            fs->newvlan |= VLAN_INSERT;
            fs->vlan = vlan_tci;
            break;
          case TCA_VLAN_ACT_MODIFY:
            fs->newvlan |= VLAN_REWRITE;
            fs->vlan = vlan_tci;
            break;
          default:
            break;
          }
      }
    else if (is_tcf_pedit(a))
      {
        u32 mask, val, offset;
        int nkeys, i;
        u8 htype;
        nkeys = tcf_pedit_nkeys(a);
        for (i = 0; i < nkeys; i++)
          {
            htype = tcf_pedit_htype(a, i);
            mask = tcf_pedit_mask(a, i);
            val = tcf_pedit_val(a, i);
            offset = tcf_pedit_offset(a, i);
            process_pedit_field(fs, val, mask, offset, htype);
          }
      }
  }
}