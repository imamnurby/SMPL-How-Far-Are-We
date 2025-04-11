@@
identifier list_head_var, loop_var;
identifier a, exts;
@@
- LIST_HEAD(list_head_var);
+ int loop_var;
@@
identifier list_head_var, loop_var;
identifier a, exts;
@@
- tcf_exts_to_list(exts, &list_head_var);
- list_for_each_entry(a, &list_head_var, list)
+ tcf_exts_for_each_action(loop_var, a, exts)