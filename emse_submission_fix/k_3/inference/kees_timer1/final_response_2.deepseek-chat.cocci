@@
expression timer, callback, data;
@@
- init_timer(&timer);
- timer.function = callback;
- timer.data = (unsigned long)data;
+ setup_timer(&timer, callback, (unsigned long)data);