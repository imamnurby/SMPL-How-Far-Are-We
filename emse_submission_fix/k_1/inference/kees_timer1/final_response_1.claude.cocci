@@
expression timer;
expression E1, E2;
@@
-init_timer(&timer);
-timer.data = (unsigned long)E1;
-timer.function = E2;
+setup_timer(&timer, E2, (unsigned long)E1);
@@
expression timer;
expression E2;
@@
-init_timer(&timer);
-timer.function = E2;
-timer.data = (unsigned long)E1;
+setup_timer(&timer, E2, (unsigned long)E1);
@@
expression timer;
expression E2;
@@
-init_timer(&timer);
-timer.function = E2;
-timer.data = 0UL;
+setup_timer(&timer, E2, 0UL);