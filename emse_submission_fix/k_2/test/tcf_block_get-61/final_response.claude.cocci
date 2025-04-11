@add_sch@
expression block, filter_list;
identifier fn;
@@
fn(...) {
<...
- tcf_block_get(block, filter_list)
+ tcf_block_get(block, filter_list, sch)
...>
}
@add_extack depends on add_sch@
expression block, filter_list;
identifier fn;
@@
fn(..., struct netlink_ext_ack *extack, ...) {
<...
- tcf_block_get(block, filter_list, sch)
+ tcf_block_get(block, filter_list, sch, extack)
...>
}


