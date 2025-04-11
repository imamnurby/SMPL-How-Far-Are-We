@@
expression port;
identifier REG;
@@
- ioread32be(port->membase + REG)
+ uart_in32(REG, port)