@@
identifier timer_var;
expression func, data;
@@
- init_timer(&timer_var);
- timer_var.function = func;
- timer_var.data = (unsigned long)data;
+ setup_timer(&timer_var, func, (unsigned long)data);


