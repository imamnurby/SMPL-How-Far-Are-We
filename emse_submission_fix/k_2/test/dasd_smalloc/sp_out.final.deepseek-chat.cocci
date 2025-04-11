@@
expression magic, cp_length, data_size, device;
expression req;
@@
- dasd_smalloc_request(magic, cp_length, data_size, device)
+ dasd_smalloc_request(magic, cp_length, data_size, device, req ? blk_mq_rq_to_pdu(req) : NULL)


