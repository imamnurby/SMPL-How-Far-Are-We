@@
expression port;
@@
- ioread32be(port->membase + ULITE_STATUS)
+ uart_in32(ULITE_STATUS, port)
@@
expression port;
@@
- ioread32be(port->membase + ULITE_RX)
+ uart_in32(ULITE_RX, port)