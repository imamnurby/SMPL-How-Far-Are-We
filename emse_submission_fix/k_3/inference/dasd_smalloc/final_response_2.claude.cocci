@@
identifier magic, length, size, dev;
expression req;
@@
- dasd_smalloc_request(magic, length, size, dev)
+ dasd_smalloc_request(magic, length, size, dev, blk_mq_rq_to_pdu(req))
@@
identifier magic, length, size, dev;
@@
- dasd_smalloc_request(magic, length, size, dev)
+ dasd_smalloc_request(magic, length, size, dev, NULL)