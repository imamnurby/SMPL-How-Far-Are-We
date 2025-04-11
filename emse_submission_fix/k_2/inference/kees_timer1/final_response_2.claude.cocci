@@
expression timer;
expression func;
expression data;
@@
(
- init_timer(&timer);
- timer.function = func;
- timer.data = data;
+ setup_timer(&timer, func, data);
|
- init_timer(&timer);
- timer.data = data;
- timer.function = func;
+ setup_timer(&timer, func, data);
|
- init_timer(&timer);
- timer.function = &func;
- timer.data = data;
+ setup_timer(&timer, &func, data);
)