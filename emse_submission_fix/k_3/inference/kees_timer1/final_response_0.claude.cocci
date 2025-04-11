@@
expression timer, fn, data;
@@
-init_timer(&timer);
-timer.function = fn;
-timer.data = data;
+setup_timer(&timer, fn, data);
@@
expression timer, fn;
@@
-init_timer(&timer);
-timer.function = fn;
+setup_timer(&timer, fn, 0UL);