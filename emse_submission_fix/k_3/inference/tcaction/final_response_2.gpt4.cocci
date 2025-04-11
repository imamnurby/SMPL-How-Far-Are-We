@@
type list_type;
identifier list_name;
iterator old_iter_macro;
expression exts;
identifier action_var, func_param;
@@
- list_type list_name;
+ int func_param;
  
<... when != list_name>
- tcf_exts_to_list(exts, &list_name);
- old_iter_macro(action_var, &list_name, list)
+ tcf_exts_for_each_action(func_param, action_var, exts)
  {
  ...
  }