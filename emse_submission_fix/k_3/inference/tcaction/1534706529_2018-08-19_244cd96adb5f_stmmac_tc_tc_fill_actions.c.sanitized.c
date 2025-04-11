static int tc_fill_actions(struct stmmac_tc_entry *entry, struct stmmac_tc_entry *frag, struct tc_cls_u32_offload *cls)
{
  struct stmmac_tc_entry *action_entry = entry;
  const struct tc_action *act;
  struct tcf_exts *exts;
  LIST_HEAD(actions);
  exts = cls->knode.exts;
  if (!tcf_exts_has_actions(exts))
    return -EINVAL;
  if (frag)
    action_entry = frag;
  tcf_exts_to_list(exts, &actions);
  list_for_each_entry(act, &actions, list)
  {

    if (is_tcf_gact_ok(act))
      {
        action_entry->val.af = 1;
        break;
      }

    if (is_tcf_gact_shot(act))
      {
        action_entry->val.rf = 1;
        break;
      }

    return -EINVAL;
  }
  return 0;
}