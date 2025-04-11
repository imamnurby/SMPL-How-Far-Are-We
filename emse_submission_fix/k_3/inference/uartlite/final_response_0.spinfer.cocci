@@
identifier I0;
expression E1;
@@
- ioread32be(E1->membase + I0) 
+ uart_in32(I0, E1) 
  ... when any
// Infered from: (./uartlite/1360605874_2013-02-11_6d53c3b71d32_uartlite_ulite_receive.{c.sanitized.c,res.c.sanitized.res.c}: ulite_receive), (./uartlite/1360605874_2013-02-11_6d53c3b71d32_uartlite_ulite_isr.{c.sanitized.c,res.c.sanitized.res.c}: ulite_isr), (./uartlite/1360605874_2013-02-11_6d53c3b71d32_uartlite_ulite_get_poll_char.{c.sanitized.c,res.c.sanitized.res.c}: ulite_get_poll_char)
// Recall: 1.00, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 3/3(100%)


// ---------------------------------------------
