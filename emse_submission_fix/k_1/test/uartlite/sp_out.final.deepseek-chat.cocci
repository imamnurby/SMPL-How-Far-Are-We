@@
expression port;
constant reg;
@@
- ioread32be(port->membase + reg)
+ uart_in32(reg, port)


