@@
expression port, reg;
@@
- ioread32be(port->membase + reg)
+ uart_in32(reg, port)