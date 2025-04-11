@@
expression magic, cplength, datasize, device;
expression req;
@@
- dasd_smalloc_request(magic, cplength, datasize, device)
+ dasd_smalloc_request(magic, cplength, datasize, device, req ? blk_mq_rq_to_pdu(req) : NULL)


