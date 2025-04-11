@@
expression E0, E1, E2;
@@
- E0 = perf_evlist__mmap(E1, E2, false); 
+ E0 = perf_evlist__mmap(E1, E2); 
// Infered from: (./perf_evlist__mmap-69/1512266437_2017-12-03_f74b9d3a1ac2_sw-clock___test__sw_clock_freq.{c,res.c}: __test__sw_clock_freq), (./perf_evlist__mmap-69/1512266437_2017-12-03_f74b9d3a1ac2_sw-clock___test__sw_clock_freq.{c,res.c}: __test__sw_clock_freq), (./perf_evlist__mmap-69/1512266437_2017-12-03_f74b9d3a1ac2_code-reading_do_test_code_reading.{c,res.c}: do_test_code_reading), (./perf_evlist__mmap-69/1512266437_2017-12-03_f74b9d3a1ac2_code-reading_do_test_code_reading.{c,res.c}: do_test_code_reading), (./perf_evlist__mmap-69/1512266437_2017-12-03_f74b9d3a1ac2_bpf_do_test.{c,res.c}: do_test), (./perf_evlist__mmap-69/1512266437_2017-12-03_f74b9d3a1ac2_bpf_do_test.{c,res.c}: do_test)
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
