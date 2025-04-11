@@
identifier action_list, action_var, priv, exts;
@@
- LIST_HEAD(action_list);
  ...
- tcf_exts_to_list(exts, &action_list);
- list_for_each_entry(action_var, &action_list, list)
+ int i;
+ tcf_exts_for_each_action(i, action_var, exts)
  {
    ...
  }