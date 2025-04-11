@@
identifier dasd_smalloc_request;
expression magic, cplength, datasize, memdev, req;
@@
(
  /* Case when `req` is present: add `blk_mq_rq_to_pdu(req)` */
- dasd_smalloc_request(magic, cplength, datasize, memdev)
+ dasd_smalloc_request(magic, cplength, datasize, memdev, blk_mq_rq_to_pdu(req))
|
  /* Case when `req` is absent: add `NULL` */
- dasd_smalloc_request(magic, cplength, datasize, memdev)
+ dasd_smalloc_request(magic, cplength, datasize, memdev, NULL)
)