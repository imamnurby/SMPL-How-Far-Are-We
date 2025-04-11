@@
identifier tcf_block_get;
expression *block, *filter_list, *extack;
expression sch;
@@
- tcf_block_get(&block, &filter_list)
+ tcf_block_get(&block, &filter_list, sch)
@@
identifier tcf_block_get;
expression *block, *filter_list, *extack;
expression sch;
@@
- tcf_block_get(&block, &filter_list, sch)
+ tcf_block_get(&block, &filter_list, sch, extack)