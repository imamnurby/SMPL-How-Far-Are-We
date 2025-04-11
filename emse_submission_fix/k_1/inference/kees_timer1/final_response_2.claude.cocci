@@
expression dev;
expression timer_var;
expression func;
expression data;
@@
- init_timer(&dev->timer_var);
- dev->timer_var.function = func;
- dev->timer_var.data = data;
+ setup_timer(&dev->timer_var, func, data);