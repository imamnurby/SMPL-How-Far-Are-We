@@
expression timer_var, callback_func, data;
@@
- init_timer(&timer_var);
- timer_var.function = callback_func;
- timer_var.data = (unsigned long)data;
+ setup_timer(&timer_var, callback_func, (unsigned long)data);