void __init mem_init(void)
{

  memset(empty_zero_page, 0, PAGE_SIZE);

  brk_end = (unsigned long)UML_ROUND_UP(sbrk(0));
  map_memory(brk_end, __pa(brk_end), uml_reserved - brk_end, 1, 1, 0);
  free_bootmem(__pa(brk_end), uml_reserved - brk_end);
  uml_reserved = brk_end;

  free_all_bootmem();
  max_low_pfn = totalram_pages;
  max_pfn = totalram_pages;
  mem_init_print_info(NULL);
  kmalloc_ok = 1;
}