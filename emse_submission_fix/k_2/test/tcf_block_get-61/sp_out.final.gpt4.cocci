@@
identifier func;
identifier q;
expression block, filter_list;
identifier sch, extack;
@@
func(...) {
  ...
- tcf_block_get(&q->block, &q->filter_list)
+ tcf_block_get(&q->block, &q->filter_list, sch, extack)
  ...
}
@@
identifier func;
identifier q;
expression block, filter_list;
identifier sch;
@@
func(...) {
  ...
- tcf_block_get(&q->block, &q->filter_list)
+ tcf_block_get(&q->block, &q->filter_list, sch)
  ...
}


