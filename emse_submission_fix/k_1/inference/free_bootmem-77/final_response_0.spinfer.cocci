@@
expression E0, E1;
@@
- if (E0 < E1)  
+ if (E0 < E1)  
  {
- free_bootmem(E0, E1 - E0); 
+ memblock_free(E0, E1 - E0); 
  }
// Infered from: (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_init_free_memmap.{c,res.c}: free_memmap), (./free_bootmem-77/1540937361_2018-10-30_2013288f7238_init_free_memmap.{c,res.c}: free_memmap)
// Recall: 1.00, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 1/1(100%)


// ---------------------------------------------
