@@
expression e1, e2, e3, e4;
@@
- dasd_smalloc_request(e1, e2, e3, e4)
+ dasd_smalloc_request(e1, e2, e3, e4, NULL)
@@
expression e1, e2, e3, e4, req;
@@
- dasd_smalloc_request(e1, e2, e3, e4)
+ dasd_smalloc_request(e1, e2, e3, e4, blk_mq_rq_to_pdu(req))