@@
expression pg, pgend;
@@
- if (pg < pgend) free_bootmem(pg, pgend - pg);
+ if (pg < pgend) memblock_free(pg, pgend - pg);