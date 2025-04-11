@@
type T;
identifier func, magic, length, data_size, device;
identifier blk_mq_rq_to_pdu, req;
@@
T func(..., struct request *req, ...) {
  ...
-  dasd_smalloc_request(magic, length, data_size, device)
+  dasd_smalloc_request(magic, length, data_size, device, blk_mq_rq_to_pdu(req))
  ...
}
@@
type T;
identifier func, magic, length, data_size, device;
@@
T func(..., ...) {
  ...
-  dasd_smalloc_request(magic, length, data_size, device)
+  dasd_smalloc_request(magic, length, data_size, device, NULL)
  ...
}