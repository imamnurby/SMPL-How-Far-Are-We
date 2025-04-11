@@
expression E1;
type T1;
identifier I, A;
iterator L;
@@
- LIST_HEAD(I);
...
- tcf_exts_to_list(E1, &I);
- L(A, &I, list) {
+ int I;
+ tcf_exts_for_each_action(I, A, E1) {
...
}