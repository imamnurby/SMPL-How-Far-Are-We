@@
expression arg1, arg2, sch, extack;
@@
(
- tcf_block_get(arg1, arg2)
+ tcf_block_get(arg1, arg2, sch)
|
- tcf_block_get(arg1, arg2, sch)
+ tcf_block_get(arg1, arg2, sch, extack)
)