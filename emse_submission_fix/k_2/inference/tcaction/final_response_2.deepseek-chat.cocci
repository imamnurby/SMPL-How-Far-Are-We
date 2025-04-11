@@
expression exts;
identifier a;
@@
- LIST_HEAD(actions);
+ int i;
  ... when != i
  <...
- tcf_exts_to_list(exts, &actions);
- list_for_each_entry(a, &actions, list)
+ tcf_exts_for_each_action(i, a, exts)
  ...>