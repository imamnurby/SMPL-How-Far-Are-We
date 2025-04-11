@@
type T;
identifier timer;
expression cb_func, cb_data;
@@
- init_timer(&timer);
- timer.function = cb_func;
? timer.data = cb_data;
+ setup_timer(&timer, cb_func, cb_data);