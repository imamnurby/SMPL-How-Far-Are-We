@@
expression port;
constant OFFSET;
@@
- ioread32be(port->membase + OFFSET)
+ uart_in32(OFFSET, port)