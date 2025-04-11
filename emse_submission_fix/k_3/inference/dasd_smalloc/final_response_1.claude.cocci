@@
expression magic, cplength, datasize, memdev;
expression req;
@@
(
- dasd_smalloc_request(magic, cplength, datasize, memdev)
+ dasd_smalloc_request(magic, cplength, datasize, memdev, NULL)
|
- dasd_smalloc_request(magic, cplength, datasize, memdev)
+ dasd_smalloc_request(magic, cplength, datasize, memdev, blk_mq_rq_to_pdu(req))
)