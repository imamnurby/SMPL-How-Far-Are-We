@rule1@
expression E1, E2;
identifier sch;
@@
- tcf_block_get(&E1, &E2)
+ tcf_block_get(&E1, &E2, sch)
@rule2@
expression E1, E2;
identifier sch, extack;
@@
- tcf_block_get(&E1, &E2, sch)
+ tcf_block_get(&E1, &E2, sch, extack)