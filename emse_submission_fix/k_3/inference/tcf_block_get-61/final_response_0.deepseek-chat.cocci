@@
expression block, filter_list, sch, extack;
@@
- tcf_block_get(&block, &filter_list)
+ tcf_block_get(&block, &filter_list, sch)
|
- tcf_block_get(&block, &filter_list, sch)
+ tcf_block_get(&block, &filter_list, sch, extack)