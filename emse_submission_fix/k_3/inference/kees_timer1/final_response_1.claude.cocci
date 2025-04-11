@@
expression tim, func, data;
@@
-init_timer(&tim);
-tim.function = func;
-tim.data = (unsigned long)data;
+setup_timer(&tim, func, (unsigned long)data);
@@
expression tim, func;
@@
-init_timer(&tim);
-tim.function = func;
-tim.data = 0UL;
+setup_timer(&tim, func, 0UL);
@@
expression tim, func;
@@
-init_timer(&tim);
-tim.function = func;
+setup_timer(&tim, func, 0UL);