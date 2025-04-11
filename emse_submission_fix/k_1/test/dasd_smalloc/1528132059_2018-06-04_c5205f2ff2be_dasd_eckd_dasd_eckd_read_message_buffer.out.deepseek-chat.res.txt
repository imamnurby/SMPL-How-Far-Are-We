init_defs_builtins: /usr/bin/../lib/coccinelle/standard.h
-----------------------------------------------------------------------
processing semantic patch file: ./dasd_smalloc/sp_out.final.deepseek-chat.cocci
with isos from: /usr/bin/../lib/coccinelle/standard.iso
-----------------------------------------------------------------------
@@
expression magic, cplength, datasize, device;
@@
- dasd_smalloc_request(magic, cplength, datasize, device)
+ dasd_smalloc_request(magic, cplength, datasize, device, NULL)



HANDLING: ./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_read_message_buffer.c.sanitized.c
-----------------------------------------------------------------------
let's go
-----------------------------------------------------------------------
-----------------------------------------------------------------------
-----------------------------------------------------------------------
rule starting on line 1 = 
-----------------------------------------------------------------------
dependencies for rule rule starting on line 1 satisfied:
binding in = []
binding relevant in = []
     transformation info returned:
          transform state: 12
               with rule_elem: -dasd_smalloc_request
                                 >>> dasd_smalloc_request(magic, cplength,
                                                          datasize, device,
                                                          NULL)
                               -(-magic-, -cplength-, -datasize-, -device-)
               with binding: [rule starting on line 1.device --> device;
                             rule starting on line 1.datasize --> (sizeof(struct
                             dasd_psf_prssd_data) + sizeof(struct
                             dasd_rssd_messages));
                             rule starting on line 1.cplength --> 1 + 1;
                             rule starting on line 1.magic --> DASD_ECKD_MAGIC]
     binding out = []
     transform one node: 12
-----------------------------------------------------------------------
Finished
-----------------------------------------------------------------------
diff = 
--- ./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_read_message_buffer.c.sanitized.c
+++ /tmp/cocci-output-93447-3d1e82-1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_read_message_buffer.c.sanitized.c
@@ -5,7 +5,9 @@ static int dasd_eckd_read_message_buffer
   struct dasd_ccw_req *cqr;
   struct ccw1 *ccw;
   int rc;
-  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1 + 1, (sizeof(struct dasd_psf_prssd_data) + sizeof(struct dasd_rssd_messages)), device);
+  cqr = dasd_smalloc_request(DASD_ECKD_MAGIC, 1 + 1,
+                             (sizeof(struct dasd_psf_prssd_data) + sizeof(struct dasd_rssd_messages)),
+                             device, NULL);
   if (IS_ERR(cqr))
     {
       DBF_EVENT_DEVID(DBF_WARNING, device->cdev, "%s", "Could not allocate read message buffer request");
Check duplication for 1 files
