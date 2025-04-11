@@
expression timer, func;
expression data;
@@
- init_timer(&timer);
- timer.function = func;
- timer.data = (unsigned long)data;
+ setup_timer(&timer, func, (unsigned long)data);