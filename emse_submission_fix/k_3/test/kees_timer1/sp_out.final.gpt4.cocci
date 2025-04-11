@@
expression timer_obj, handler_func, timer_data;
@@
- init_timer(&timer_obj);
- timer_obj.function = handler_func;
- timer_obj.data = timer_data;
+ setup_timer(&timer_obj, handler_func, timer_data);


