@@
expression timer, function;
expression data;
@@
- init_timer(&timer);
- timer.function = function;
- timer.data = (unsigned long)data;
+ setup_timer(&timer, function, (unsigned long)data);