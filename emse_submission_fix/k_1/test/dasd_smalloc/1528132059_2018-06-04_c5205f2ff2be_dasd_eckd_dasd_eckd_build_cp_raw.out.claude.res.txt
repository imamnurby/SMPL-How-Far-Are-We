init_defs_builtins: /usr/bin/../lib/coccinelle/standard.h
-----------------------------------------------------------------------
processing semantic patch file: ./dasd_smalloc/sp_out.final.claude.cocci
with isos from: /usr/bin/../lib/coccinelle/standard.iso
-----------------------------------------------------------------------
@@
expression magic, cplength, datasize, device;
expression E;
@@
- dasd_smalloc_request(magic, cplength, datasize, device)
+ dasd_smalloc_request(magic, cplength, datasize, device, E)



warning: rule starting on line 1: metavariable E not used in the - or context code
error: rule starting on line 1: E appears only in + code
