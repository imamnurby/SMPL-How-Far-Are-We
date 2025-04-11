@@
expression E1, E0;
@@
- free_bootmem(E0, E1); 
+ memblock_free(E0, E1); 
// Infered from: (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_xhci-dbc_xdbc_free_ring.{c,res.c}: xdbc_free_ring), (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_xhci-dbc_xdbc_free_ring.{c,res.c}: xdbc_free_ring), (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_tce_64_free_tce_table.{c,res.c}: free_tce_table), (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_tce_64_free_tce_table.{c,res.c}: free_tce_table), (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_init_free_memmap.{c,res.c}: free_memmap), (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_init_free_memmap.{c,res.c}: free_memmap)
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
