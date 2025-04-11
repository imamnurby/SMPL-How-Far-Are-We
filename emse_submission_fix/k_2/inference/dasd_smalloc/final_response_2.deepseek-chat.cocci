@@
expression magic, cplength, datasize, device;
identifier req;
@@
dasd_smalloc_request(
  magic,
  cplength,
  datasize,
  device
+ , \( NULL \| blk_mq_rq_to_pdu(req) \)
  )