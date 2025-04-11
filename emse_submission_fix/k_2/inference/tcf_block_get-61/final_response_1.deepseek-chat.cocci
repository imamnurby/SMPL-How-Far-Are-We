@@
expression sch, q, err;
@@
static int (...)(
  struct Qdisc *sch,
  ...)
{
  <+...
- err = tcf_block_get(&q->block, &q->filter_list);
+ err = tcf_block_get(&q->block, &q->filter_list, sch);
  ...+>
}
@@
expression sch, q, err;
struct netlink_ext_ack *extack;
@@
static int (...)(
  struct Qdisc *sch,
  ...,
  struct netlink_ext_ack *extack,
  ...)
{
  <+...
- err = tcf_block_get(&q->block, &q->filter_list, sch);
+ err = tcf_block_get(&q->block, &q->filter_list, sch, extack);
  ...+>
}