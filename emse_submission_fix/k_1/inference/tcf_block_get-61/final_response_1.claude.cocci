@rule1@
expression E1, E2;
identifier FN;
expression SCH;
@@
FN(...) {
<...
- tcf_block_get(E1, E2)
+ tcf_block_get(E1, E2, SCH)
...>
}
@rule2@
expression E1, E2, E3;
identifier FN;
expression EXTACK;
@@
FN(..., struct netlink_ext_ack *EXTACK, ...) {
<...
- tcf_block_get(E1, E2, E3)
+ tcf_block_get(E1, E2, E3, EXTACK)
...>
}