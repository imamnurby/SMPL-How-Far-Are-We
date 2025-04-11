@@
identifier ioread = "ioread32be";
identifier uart_in = "uart_in32";
identifier membase_field = "membase";
identifier reg_macro;
expression port;
@@
- ioread(port->membase_field + reg_macro)
+ uart_in(reg_macro, port)