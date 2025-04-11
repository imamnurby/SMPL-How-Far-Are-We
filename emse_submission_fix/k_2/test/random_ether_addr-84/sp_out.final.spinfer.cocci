@@
identifier I1;
expression E0;
@@
- random_ether_addr(E0->I1); 
+ eth_random_addr(E0->I1); 
// Infered from: (./random_ether_addr-84/1529689860_2018-06-22_6c1f0a1ffb7c_qlcnic_sriov_common_qlcnic_sriov_init.{c.sanitized.c,res.c.sanitized.res.c}: qlcnic_sriov_init), (./random_ether_addr-84/1529689860_2018-06-22_6c1f0a1ffb7c_gemini_gemini_ethernet_port_probe.{c.sanitized.c,res.c.sanitized.res.c}: gemini_ethernet_port_probe)
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
