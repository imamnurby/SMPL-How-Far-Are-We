init_defs_builtins: /usr/bin/../lib/coccinelle/standard.h
-----------------------------------------------------------------------
processing semantic patch file: ./dasd_smalloc/sp_out.final.gpt4.cocci
with isos from: /usr/bin/../lib/coccinelle/standard.iso
-----------------------------------------------------------------------
@@
expression e1, e2, e3, e4;
expression new_arg; // This matches the new fifth argument (e.g., NULL, blk_mq_rq_to_pdu(...))
@@
- dasd_smalloc_request(e1, e2, e3, e4)
+ dasd_smalloc_request(e1, e2, e3, e4, new_arg)



warning: rule starting on line 1: metavariable new_arg not used in the - or context code
error: rule starting on line 1: new_arg appears only in + code
