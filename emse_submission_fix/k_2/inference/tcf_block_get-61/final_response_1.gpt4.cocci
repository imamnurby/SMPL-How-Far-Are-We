@@
type T;
identifier I;
@@
T I(...) {
  ...
  expression e1, e2, e3;
  identifier extack;
  ...
- tcf_block_get(e1, e2)
+ tcf_block_get(e1, e2, e3)
  ...
}
@@
type T;
identifier I;
@@
T I(...) {
  ...
  expression e1, e2, e3;
  identifier extack;
  ...
- tcf_block_get(e1, e2, e3)
+ tcf_block_get(e1, e2, e3, extack)
  ...
}