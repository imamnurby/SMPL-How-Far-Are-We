@@
expression timer;
expression func;
expression data;
@@
- init_timer(&timer);
- timer.function = func;
- timer.data = data;
+ setup_timer(&timer, func, data);