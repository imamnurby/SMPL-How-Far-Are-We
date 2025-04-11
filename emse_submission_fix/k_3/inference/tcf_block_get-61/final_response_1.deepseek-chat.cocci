@@
identifier x;
expression sch, extack;
@@
- tcf_block_get(&x->block, &x->filter_list)
+ tcf_block_get(&x->block, &x->filter_list, sch)
@@
identifier x;
expression sch;
struct netlink_ext_ack *extack;
@@
- tcf_block_get(&x->block, &x->filter_list, sch)
+ tcf_block_get(&x->block, &x->filter_list, sch, extack)