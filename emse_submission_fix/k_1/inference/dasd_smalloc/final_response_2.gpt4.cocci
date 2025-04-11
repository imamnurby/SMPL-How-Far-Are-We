@@
expression e_magic, e_cplength, e_datasize, e_device;
expression e_extra;
@@
- dasd_smalloc_request(e_magic, e_cplength, e_datasize, e_device)
+ dasd_smalloc_request(e_magic, e_cplength, e_datasize, e_device, e_extra)
@@
expression e_magic, e_cplength, e_datasize, e_device;
@@
- dasd_smalloc_request(e_magic, e_cplength, e_datasize, e_device)
+ dasd_smalloc_request(e_magic, e_cplength, e_datasize, e_device, NULL)