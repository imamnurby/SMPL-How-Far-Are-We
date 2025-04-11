@@
identifier port;
identifier CONSTANT;
@@
- ioread32be(port->membase + CONSTANT)
+ uart_in32(CONSTANT, port)