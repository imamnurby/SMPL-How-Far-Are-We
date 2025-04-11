@@
@@
- init_timer(&dev->timer); 
- dev->timer.function = isdn_timer_funct; 
+ setup_timer(&dev->timer, isdn_timer_funct, 0UL); 
// Infered from: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_isdn_common_isdn_init.{c.sanitized.c,res.c.sanitized.res.c}: isdn_init)
// Recall: 0.27, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
@@
type T1;
expression E0;
@@
- init_timer(&E0->timer); 
- E0->timer.function = dasd_device_timeout; 
- E0->timer.data = (T1 )E0; 
+ setup_timer(&E0->timer, dasd_device_timeout, (T1 )E0); 
// Infered from: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_dasd_dasd_alloc_device.{c.sanitized.c,res.c.sanitized.res.c}: dasd_alloc_device)
// Recall: 0.36, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
@@
type T1;
expression E0;
@@
- init_timer(&E0->eh_timer); 
- E0->eh_timer.data = (T1 )E0; 
- E0->eh_timer.function = fas216_eh_timer; 
+ setup_timer(&E0->eh_timer, fas216_eh_timer, (T1 )E0); 
// Infered from: (./kees_timer1/1508184939_2017-10-16_b9eaf1872222_fas216_fas216_init.{c.sanitized.c,res.c.sanitized.res.c}: fas216_init)
// Recall: 0.36, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 3 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 3/3(100%)


// ---------------------------------------------
