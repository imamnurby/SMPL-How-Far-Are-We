@@
expression addr, size;
@@
- free_bootmem(addr, size)
+ memblock_free(addr, size)