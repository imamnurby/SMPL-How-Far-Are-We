@@
expression E1, E2, E3, E4;
identifier F = dasd_smalloc_request;
@@
- F(E1, E2, E3, E4)
+ F(E1, E2, E3, E4, NULL)
@@
expression E1, E2, E3, E4, E5;
identifier F = dasd_smalloc_request;
identifier Req;
@@
- F(E1, E2, E3, E4)
+ F(E1, E2, E3, E4, blk_mq_rq_to_pdu(Req))