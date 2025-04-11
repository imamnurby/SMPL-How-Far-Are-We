init_defs_builtins: /usr/bin/../lib/coccinelle/standard.h
-----------------------------------------------------------------------
processing semantic patch file: ./dasd_smalloc/sp_out.final.gpt4.cocci
with isos from: /usr/bin/../lib/coccinelle/standard.iso
-----------------------------------------------------------------------
@@
identifier dasd_smalloc_request;
identifier device;
expression type, count, size;
@@
- dasd_smalloc_request(type, count, size, device)
+ dasd_smalloc_request(type, count, size, device, NULL)



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
                                 >>> dasd_smalloc_request(type, count, size,
                                                          device, NULL)
                               -(-type-, -count-, -size-, -device-)
               with binding: [rule starting on line 1.device --> id device;
                             rule starting on line 1.size --> (sizeof(struct
                             dasd_psf_prssd_data) + sizeof(struct
                             dasd_rssd_messages));
                             rule starting on line 1.count --> 1 + 1;
                             rule starting on line 1.type --> DASD_ECKD_MAGIC;
                             rule starting on line 1.dasd_smalloc_request --> id dasd_smalloc_request]
          transform state: 75
               with rule_elem: -dasd_smalloc_request
                                 >>> dasd_smalloc_request(type, count, size,
                                                          device, NULL)
                               -(-type-, -count-, -size-, -device-)
               with binding: [rule starting on line 1.device --> id rc;
                             rule starting on line 1.size --> "Reading messages failed with rc=%d\n";
                             rule starting on line 1.count --> device->cdev;
                             rule starting on line 1.type --> DBF_WARNING;
                             rule starting on line 1.dasd_smalloc_request --> id DBF_EVENT_DEVID]
     binding out = []
     transform one node: 75
     transform one node: 12
-----------------------------------------------------------------------
Finished
-----------------------------------------------------------------------
diff = 
--- ./dasd_smalloc/1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_read_message_buffer.c.sanitized.c
+++ /tmp/cocci-output-92708-309b40-1528132059_2018-06-04_c5205f2ff2be_dasd_eckd_dasd_eckd_read_message_buffer.c.sanitized.c
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
@@ -57,7 +59,8 @@ retry:
       goto retry;
     }
   else
-    DBF_EVENT_DEVID(DBF_WARNING, device->cdev, "Reading messages failed with rc=%d\n", rc);
+    DBF_EVENT_DEVID(DBF_WARNING, device->cdev,
+                    "Reading messages failed with rc=%d\n", rc, NULL);
   dasd_sfree_request(cqr, cqr->memdev);
   return rc;
 }
\ No newline at end of file
Check duplication for 1 files
