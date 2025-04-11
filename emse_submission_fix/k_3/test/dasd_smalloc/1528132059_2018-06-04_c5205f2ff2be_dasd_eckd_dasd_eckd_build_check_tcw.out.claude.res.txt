init_defs_builtins: /usr/bin/../lib/coccinelle/standard.h
-----------------------------------------------------------------------
processing semantic patch file: ./dasd_smalloc/sp_out.final.claude.cocci
with isos from: /usr/bin/../lib/coccinelle/standard.iso
-----------------------------------------------------------------------
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



warning: rule starting on line 1: metavariable req not used in the - or context code
error: rule starting on line 1: req appears only in + code
