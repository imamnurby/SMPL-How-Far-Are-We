@@
type timer_struct = struct timer_list;
identifier T;
expression F, D;
@@
- init_timer(&T);
- T.function = F;
- T.data = D;
+ setup_timer(&T, F, D);
@@
type timer_struct = struct timer_list;
identifier T;
expression F;
@@
- init_timer(&T);
- T.function = F;
+ setup_timer(&T, F, 0UL);