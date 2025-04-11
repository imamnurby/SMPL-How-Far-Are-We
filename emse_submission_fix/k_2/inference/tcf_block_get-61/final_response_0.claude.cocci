@@
expression q_block, q_filter_list;
identifier sch;
@@
-tcf_block_get(q_block, q_filter_list)
+tcf_block_get(q_block, q_filter_list, sch)
@@
expression q_block, q_filter_list;
identifier sch, extack;
@@
-tcf_block_get(q_block, q_filter_list, sch)
+tcf_block_get(q_block, q_filter_list, sch, extack)