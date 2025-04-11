@@
type T;
identifier variable;
identifier sch;
@@
- tcf_block_get(&variable->block, &variable->filter_list)
+ tcf_block_get(&variable->block, &variable->filter_list, sch)
@@
type T;
identifier variable;
identifier sch, extack;
@@
- tcf_block_get(&variable->block, &variable->filter_list, sch)
+ tcf_block_get(&variable->block, &variable->filter_list, sch, extack)