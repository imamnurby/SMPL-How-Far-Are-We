@add_sch@
expression q;
expression E;
@@
- tcf_block_get(&q->block, &q->filter_list)
+ tcf_block_get(&q->block, &q->filter_list, E)
@add_extack@
expression q, sch, extack;
@@
- tcf_block_get(&q->block, &q->filter_list, sch)
+ tcf_block_get(&q->block, &q->filter_list, sch, extack)