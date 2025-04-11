@@
identifier func_to_replace = early_iounmap;
expression args;
@@
- func_to_replace(args)
+ early_memunmap(args)