@@
type list_head_type;
identifier actions;
@@
- LIST_HEAD(actions);
+ int i;
@@
expression exts;
identifier actions;
@@
- tcf_exts_to_list(exts, &actions);
@@
identifier actions;
iterator list_for_each_entry;
identifier a;
@@
- list_for_each_entry(a, &actions, list)
+ tcf_exts_for_each_action(i, a, exts)


