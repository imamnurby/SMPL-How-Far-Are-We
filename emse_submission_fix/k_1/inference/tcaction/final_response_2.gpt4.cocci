@@
identifier i, a, exts, actions;
@@
- LIST_HEAD(actions);
...
- tcf_exts_to_list(exts, &actions);
- list_for_each_entry(a, &actions, list)
+ int i;
+ tcf_exts_for_each_action(i, a, exts)