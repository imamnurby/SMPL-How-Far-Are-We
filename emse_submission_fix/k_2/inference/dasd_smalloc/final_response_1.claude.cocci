@@
expression e1, e2, e3, e4;
identifier req;
@@
(
- dasd_smalloc_request(e1, e2, e3, e4)
+ dasd_smalloc_request(e1, e2, e3, e4, blk_mq_rq_to_pdu(req))
|
- dasd_smalloc_request(e1, e2, e3, e4)
+ dasd_smalloc_request(e1, e2, e3, e4, NULL)
)
@@
expression e1, e2, e3, e4;
@@
// Handle cases where IS_ERR is checking the result
// but we don't have access to a req variable
  IS_ERR(
- dasd_smalloc_request(e1, e2, e3, e4)
+ dasd_smalloc_request(e1, e2, e3, e4, NULL)
  )