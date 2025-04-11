@@
identifier timer_var;
expression timer_func, timer_data;
@@
- init_timer(&timer_var);
- timer_var.function = timer_func;
- timer_var.data = timer_data;
+ setup_timer(&timer_var, timer_func, timer_data);