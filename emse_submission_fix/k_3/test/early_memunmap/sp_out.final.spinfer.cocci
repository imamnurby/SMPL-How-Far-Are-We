@@
expression E0, E1;
@@
- early_iounmap(E0, E1); 
+ early_memunmap(E0, E1); 
// Infered from: (./early_memunmap/1424769208_2015-02-24_8d4a40bc0651_e820_parse_e820_ext.{c,res.c}: parse_e820_ext), (./early_memunmap/1424769208_2015-02-24_8d4a40bc0651_e820_parse_e820_ext.{c,res.c}: parse_e820_ext), (./early_memunmap/1424769208_2015-02-24_8d4a40b_setup_relocate_initrd.{c,res.c}: relocate_initrd), (./early_memunmap/1424769208_2015-02-24_8d4a40b_setup_relocate_initrd.{c,res.c}: relocate_initrd)
// Recall: 1.00, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 2/2(100%)


// ---------------------------------------------
