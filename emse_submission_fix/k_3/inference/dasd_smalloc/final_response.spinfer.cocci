@@
identifier I0, I5, I1, I2, I3;
expression E6, E4, E7, E8;
@@
  struct dasd_ccw_req *I0(struct dasd_device *I1, struct dasd_block *I2,                         struct request *I3)  
  {
  ...
- E4 = dasd_smalloc_request(I5, E6, E7, E8); 
+ E4 = dasd_smalloc_request(I5, E6, E7, E8, blk_mq_rq_to_pdu(I3)); 
  ...
  }
// Infered from: (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_fba_dasd_fba_build_cp_regular.{c.sanitized.c,res.c.sanitized.res.c}: dasd_fba_build_cp_regular), (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_fba_dasd_fba_build_cp_discard.{c.sanitized.c,res.c.sanitized.res.c}: dasd_fba_build_cp_discard), (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_diag_dasd_diag_build_cp.{c.sanitized.c,res.c.sanitized.res.c}: dasd_diag_build_cp)
// Recall: 0.17, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
@@
identifier I1;
expression E0, E4, E2, E3;
@@
- E0 = dasd_smalloc_request(I1, E2, E3, E4); 
+ E0 = dasd_smalloc_request(DASD_ECKD_MAGIC, E2, E3, E4, NULL); 
// Infered from: (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_symm_io.{c.sanitized.c,res.c.sanitized.res.c}: dasd_symm_io), (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_steal_lock.{c.sanitized.c,res.c.sanitized.res.c}: dasd_eckd_steal_lock), (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_analysis_ccw.{c.sanitized.c,res.c.sanitized.res.c}: dasd_eckd_analysis_ccw)
// False positives: (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_diag_dasd_diag_build_cp.res.c.sanitized.res.c: dasd_diag_build_cp), (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_fba_dasd_fba_build_cp_discard.res.c.sanitized.res.c: dasd_fba_build_cp_discard), (./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_fba_dasd_fba_build_cp_regular.res.c.sanitized.res.c: dasd_fba_build_cp_regular)
// Recall: 0.75, Precision: 0.75, Matching recall: 0.75

// ---------------------------------------------
// Final metrics (for the combined 2 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 0.83, Precision: 0.83
// -- General --
// Functions fully changed: 4/6(66%)

/*
Functions where the patch produced incorrect changes:
 - ./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_diag_dasd_diag_build_cp.c.sanitized.c: dasd_diag_build_cp
 - ./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_fba_dasd_fba_build_cp_regular.c.sanitized.c: dasd_fba_build_cp_regular
*/

// ---------------------------------------------
