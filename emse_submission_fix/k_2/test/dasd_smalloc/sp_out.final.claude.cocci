@@
expression magic_type, count, datasize, memdev;
expression req;
@@
(
// Case 1: Context where req is available (typically in req-handling functions)
- dasd_smalloc_request(magic_type, count, datasize, memdev)
+ dasd_smalloc_request(magic_type, count, datasize, memdev, blk_mq_rq_to_pdu(req))
|
// Case 2: Context where req is not available
- dasd_smalloc_request(magic_type, count, datasize, memdev)
+ dasd_smalloc_request(magic_type, count, datasize, memdev, NULL)
)


