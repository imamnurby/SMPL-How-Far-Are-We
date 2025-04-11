@@
expression block, filter_list;
identifier sch;
struct netlink_ext_ack *extack;
@@
- tcf_block_get(&block, &filter_list)
+ tcf_block_get(&block, &filter_list, sch)
@@
expression block, filter_list;
identifier sch;
struct netlink_ext_ack *extack;
position p;
@@
tcf_block_get@p(&block, &filter_list, sch)
{ ... when != extack
+ tcf_block_get(&block, &filter_list, sch, extack)
... when any
}