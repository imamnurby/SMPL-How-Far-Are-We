@@
identifier old_func;
expression size_expr;
@@
- early_iounmap(old_func, size_expr);
+ early_memunmap(old_func, size_expr);