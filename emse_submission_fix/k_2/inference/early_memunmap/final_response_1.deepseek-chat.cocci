@@
expression sdata, data_len;
@@
- early_iounmap(sdata, data_len)
+ early_memunmap(sdata, data_len)