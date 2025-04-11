@@
type T1;
expression E0;
@@
- init_timer(&E0->eh_timer); 
- E0->eh_timer.data = (T1 )E0; 
- E0->eh_timer.function = fas216_eh_timer; 
+ setup_timer(&E0->eh_timer, fas216_eh_timer, (T1 )E0); 
// Infered from: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_fas216_fas216_init.{c.sanitized.c,res.c.sanitized.res.c}: fas216_init)
// Recall: 0.18, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
@@
identifier I1, I2;
type T3;
expression E0;
@@
- init_timer(&E0->I1); 
- E0->I1.function = I2; 
- E0->I1.data = (T3 )E0; 
+ setup_timer(&E0->I1, I2, (T3 )E0); 
// Infered from: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_dasd_dasd_alloc_device.{c.sanitized.c,res.c.sanitized.res.c}: dasd_alloc_device), (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_bfad_bfad_init_timer.{c.sanitized.c,res.c.sanitized.res.c}: bfad_init_timer)
// Recall: 0.36, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
@@
identifier I1;
expression E0;
@@
- init_timer(&E0); 
- E0.function = I1; 
+ setup_timer(&E0, I1, 0UL); 
// Infered from: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_nmi_init_nmi_wdt.{c.sanitized.c,res.c.sanitized.res.c}: init_nmi_wdt), (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_isdn_common_isdn_init.{c.sanitized.c,res.c.sanitized.res.c}: isdn_init)
// False positives: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_bfad_bfad_init_timer.res.c.sanitized.res.c: bfad_init_timer), (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_dasd_dasd_alloc_device.res.c.sanitized.res.c: dasd_alloc_device)
// Recall: 0.45, Precision: 0.83, Matching recall: 0.71

// ---------------------------------------------
@@
type T1;
expression E0;
@@
- init_timer(&E0->eternal_timer); 
+ setup_timer(&E0->eternal_timer, &arcmsr_request_device_map, (T1 )E0); 
  ...
- E0->eternal_timer.data = (T1 )E0; 
- E0->eternal_timer.function = &arcmsr_request_device_map; 
// Infered from: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_arcmsr_hba_arcmsr_probe.{c.sanitized.c,res.c.sanitized.res.c}: arcmsr_probe)
// Recall: 0.18, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 4 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 6/6(100%)


// ---------------------------------------------
