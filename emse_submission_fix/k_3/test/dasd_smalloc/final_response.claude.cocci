@@ 
expression magic, count1, count2, device;
expression req;
@@
(
// When we have a request parameter available in the context - use it
- dasd_smalloc_request(magic, count1, count2, device)
+ dasd_smalloc_request(magic, count1, count2, device, blk_mq_rq_to_pdu(req))
|
// For all other cases - use NULL
- dasd_smalloc_request(magic, count1, count2, device)
+ dasd_smalloc_request(magic, count1, count2, device, NULL)
)


