@@
expression timer, function;
identifier data;
@@
- init_timer(&timer);
- timer.function = function;
- timer.data = (unsigned long)data;
+ setup_timer(&timer, function, (unsigned long)data);
@@
expression timer, function;
@@
- init_timer(&timer);
- timer.function = function;
- timer.data = 0UL;
+ setup_timer(&timer, function, 0UL);


