@@
expression e; // Represents `tcf_exts` variable
identifier f; // Represents iterator variable for iteration, e.g., `a`
@
- LIST_HEAD(actions);
+ int i;
... when != tcf_exts_for_each_action(...)
    when != i
- tcf_exts_to_list(e, &actions);
- list_for_each_entry(f, &actions, list)
+ tcf_exts_for_each_action(i, f, e)