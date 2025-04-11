static int qede_parse_actions(struct qede_dev *edev,
			      struct tcf_exts *exts)
{
	int rc = -EINVAL, num_act = 0;
	const struct tc_action *a;
	bool is_drop = false;
	LIST_HEAD(actions);
	if (!tcf_exts_has_actions(exts)) {
		DP_NOTICE(edev, "No tc actions received\n");
		return rc;
	}
	tcf_exts_to_list(exts, &actions);
	list_for_each_entry(a, &actions, list) {
		num_act++;
		if (is_tcf_gact_shot(a))
			is_drop = true;
	}
	if (num_act == 1 && is_drop)
		return 0;
	return rc;
}