@@
expression magic, length, datasize;
identifier device;
@@
- dasd_smalloc_request(magic, length, datasize, device)
+ dasd_smalloc_request(magic, length, datasize, device, NULL)
@@
expression magic, length, datasize;
identifier device;
expression req;
@@
- dasd_smalloc_request(magic, length, datasize, device)
+ dasd_smalloc_request(magic, length, datasize, device, blk_mq_rq_to_pdu(req))