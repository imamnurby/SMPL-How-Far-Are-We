@@
expression E0, E1;
iterator name list_for_each_entry;
iterator name tcf_exts_for_each_action;
@@
- LIST_HEAD(actions); 
+ int i; 
  ...
- tcf_exts_to_list(E0, &actions); 
- list_for_each_entry (E1, &actions, list)  
+ tcf_exts_for_each_action (i, E1, E0)  
  {
  ...
  }
// Infered from: (./tcaction/1534706529_2018-08-19_244cd96adb5f_en_tc_parse_tc_fdb_actions.{c.sanitized.c,res.c.sanitized.res.c}: parse_tc_fdb_actions)
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
