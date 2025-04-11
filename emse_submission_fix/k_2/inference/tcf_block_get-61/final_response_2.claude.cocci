@rule1@
identifier block, filter_list;
identifier sch;
identifier extack;
@@
- tcf_block_get(&block, &filter_list, sch)
+ tcf_block_get(&block, &filter_list, sch, extack)
@rule2@
identifier block, filter_list;
identifier sch;
@@
- tcf_block_get(&block, &filter_list)
+ tcf_block_get(&block, &filter_list, sch)
@rule3@
identifier block, filter_list;
@@
- tcf_block_get(&block, &filter_list, ...)
+ tcf_block_get(&block, &filter_list, ...)