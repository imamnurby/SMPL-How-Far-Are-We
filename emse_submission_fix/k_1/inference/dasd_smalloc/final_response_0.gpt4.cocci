@@
identifier dasd_smalloc_request;
identifier device;
expression type, count, size;
@@
- dasd_smalloc_request(type, count, size, device)
+ dasd_smalloc_request(type, count, size, device, NULL)