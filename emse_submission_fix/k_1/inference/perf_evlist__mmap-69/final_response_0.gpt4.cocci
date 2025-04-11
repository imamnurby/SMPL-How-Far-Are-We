@@
identifier mmap_func = perf_evlist__mmap;
expression evlist, pages, extra_arg;
@@
- mmap_func(evlist, pages, extra_arg)
+ mmap_func(evlist, pages)