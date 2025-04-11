@@
identifier I1;
expression E0, E2, E3, E4;
@@
- E0 = dasd_smalloc_request(I1, E2, E3, E4); 
+ E0 = dasd_smalloc_request(DASD_ECKD_MAGIC, E2, E3, E4, NULL); 
// Infered from: (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_analysis_ccw.{c.sanitized.c,res.c.sanitized.res.c}: dasd_eckd_analysis_ccw)
// False positives: (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_diag_dasd_diag_build_cp.res.c.sanitized.res.c: dasd_diag_build_cp)
// Recall: 0.75, Precision: 0.75, Matching recall: 0.75

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 0.75, Precision: 0.75
// -- General --
// Functions fully changed: 1/2(50%)

/*
Functions where the patch produced incorrect changes:
 - ./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_diag_dasd_diag_build_cp.c.sanitized.c: dasd_diag_build_cp
*/

// ---------------------------------------------
