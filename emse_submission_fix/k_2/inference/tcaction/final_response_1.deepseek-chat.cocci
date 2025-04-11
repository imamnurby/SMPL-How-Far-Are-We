@@
identifier a;
identifier exts;
statement S;
@@
- LIST_HEAD(actions);
+ int i;
  ... when != i
  if (!tcf_exts_has_actions(exts))
    return ...;
- tcf_exts_to_list(exts, &actions);
- list_for_each_entry(a, &actions, list)
+ tcf_exts_for_each_action(i, a, exts)
  S