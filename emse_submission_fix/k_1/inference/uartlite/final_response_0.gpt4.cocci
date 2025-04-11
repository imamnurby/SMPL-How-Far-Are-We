@@
identifier base;
expression offset;
identifier port;
@@
- ioread32be(port->base + offset)
+ uart_in32(offset, port)