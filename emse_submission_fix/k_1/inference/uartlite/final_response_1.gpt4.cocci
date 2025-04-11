@@
expression value, membase_expr;
identifier port;
@@
- ioread32be(\(membase_expr\) + value)
+ uart_in32(value, port)