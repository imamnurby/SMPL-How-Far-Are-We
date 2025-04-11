@@
identifier port;
identifier membase;
identifier offset;
@@
- ioread32be(port->membase + offset)
+ uart_in32(offset, port)