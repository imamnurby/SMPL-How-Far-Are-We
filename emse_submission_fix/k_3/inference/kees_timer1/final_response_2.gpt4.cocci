@@
identifier E; // Timer variable
expression T, D; // Timer callback function and data
@@
- init_timer(&E);
- E.function = T;
- E.data = (unsigned long)D;
+ setup_timer(&E, T, (unsigned long)D);