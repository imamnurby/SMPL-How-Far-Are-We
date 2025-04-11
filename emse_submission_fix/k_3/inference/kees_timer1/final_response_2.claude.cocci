@@
expression T, F, D;
@@
-init_timer(&T);
-T.function = F;
-T.data = D;
+setup_timer(&T, F, D);
@@
expression T, F;
@@
-init_timer(&T);
-T.function = F;
+setup_timer(&T, F, 0UL);