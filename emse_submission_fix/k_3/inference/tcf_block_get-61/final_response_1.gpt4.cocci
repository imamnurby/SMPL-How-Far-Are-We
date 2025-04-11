@@
identifier block, filter_list;
identifier func =~ "tcf_block_get";
expression sch, extack;
@@
(
- func(&block, &filter_list)
+ func(&block, &filter_list, sch)
|
- func(&block, &filter_list, sch)
+ func(&block, &filter_list, sch, extack)
)