@@
expression timer, function, data;
@@
- init_timer(&timer);
- timer.function = function;
- timer.data = data;
+ setup_timer(&timer, function, data);