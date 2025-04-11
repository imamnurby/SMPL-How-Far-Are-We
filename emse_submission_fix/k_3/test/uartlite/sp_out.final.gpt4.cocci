@@
expression E;        // Matches the `port` structure (base address struct)
constant C;          // Matches the constant offset like ULITE_STATUS or ULITE_RX
@@
- ioread32be(E->membase + C)
+ uart_in32(C, E)


