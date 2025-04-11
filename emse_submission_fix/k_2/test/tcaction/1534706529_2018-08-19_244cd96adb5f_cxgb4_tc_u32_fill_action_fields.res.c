static int fill_action_fields(struct adapter *adap,
			      struct ch_filter_specification *fs,
			      struct tc_cls_u32_offload *cls)
{
	unsigned int num_actions = 0;
	const struct tc_action *a;
	struct tcf_exts *exts;
	int i;
	exts = cls->knode.exts;
	if (!tcf_exts_has_actions(exts))
		return -EINVAL;
	tcf_exts_for_each_action(i, a, exts) {
		/* Don't allow more than one action per rule. */
		if (num_actions)
			return -EINVAL;
		/* Drop in hardware. */
		if (is_tcf_gact_shot(a)) {
			fs->action = FILTER_DROP;
			num_actions++;
			continue;
		}
		/* Re-direct to specified port in hardware. */
		if (is_tcf_mirred_egress_redirect(a)) {
			struct net_device *n_dev, *target_dev;
			bool found = false;
			unsigned int i;
			target_dev = tcf_mirred_dev(a);
			for_each_port(adap, i) {
				n_dev = adap->port[i];
				if (target_dev == n_dev) {
					fs->action = FILTER_SWITCH;
					fs->eport = i;
					found = true;
					break;
				}
			}
			/* Interface doesn't belong to any port of
			 * the underlying hardware.
			 */
			if (!found)
				return -EINVAL;
			num_actions++;
			continue;
		}
		/* Un-supported action. */
		return -EINVAL;
	}
	return 0;
}