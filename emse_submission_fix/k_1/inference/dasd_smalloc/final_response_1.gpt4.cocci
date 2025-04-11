@@
expression magic, cplength, datasize, device, fifth_arg;
@@
- dasd_smalloc_request(magic, cplength, datasize, device)
+ dasd_smalloc_request(magic, cplength, datasize, device, NULL)
@@
expression magic, cplength, datasize, device;
identifier req;
@@
- dasd_smalloc_request(magic, cplength, datasize, device)
+ dasd_smalloc_request(magic, cplength, datasize, device, blk_mq_rq_to_pdu(req))