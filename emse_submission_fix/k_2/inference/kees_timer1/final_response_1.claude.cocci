@@
expression timer;
expression func;
expression data;
@@
-init_timer(&timer);
-timer.function = func;
-timer.data = data;
+setup_timer(&timer, func, data);
@@
expression timer;
expression func;
@@
-init_timer(&timer);
-timer.function = func;
+setup_timer(&timer, func, 0UL);