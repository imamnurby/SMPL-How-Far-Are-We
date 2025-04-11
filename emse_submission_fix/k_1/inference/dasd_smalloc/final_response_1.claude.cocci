@@
expression magic, cplength, datasize, dev;
expression req;
@@
- dasd_smalloc_request(magic, cplength, datasize, dev)
+ dasd_smalloc_request(magic, cplength, datasize, dev, req ? blk_mq_rq_to_pdu(req) : NULL)
@@
expression magic, cplength, datasize, dev;
@@
- dasd_smalloc_request(magic, cplength, datasize, dev)
+ dasd_smalloc_request(magic, cplength, datasize, dev, NULL)