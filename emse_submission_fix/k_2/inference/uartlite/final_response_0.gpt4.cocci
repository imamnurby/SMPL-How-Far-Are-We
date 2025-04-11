@@
identifier port;
identifier REGISTER;
expression MEMBASE;
@@
- ioread32be(MEMBASE + REGISTER)
+ uart_in32(REGISTER, port)