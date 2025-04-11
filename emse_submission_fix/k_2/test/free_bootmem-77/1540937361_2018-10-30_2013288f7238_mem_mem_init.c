void __init mem_init(void) {
  /* clear the zero-page */
  memset(empty_zero_page, 0, PAGE_SIZE);
  /* Map in the area just after the brk now that kmalloc is about
   * to be turned on.
   */
  brk_end = (unsigned long)UML_ROUND_UP(sbrk(0));
  map_memory(brk_end, __pa(brk_end), uml_reserved - brk_end, 1, 1, 0);
  free_bootmem(__pa(brk_end), uml_reserved - brk_end);
  uml_reserved = brk_end;
  /* this will put all low memory onto the freelists */
  free_all_bootmem();
  max_low_pfn = totalram_pages;
  max_pfn = totalram_pages;
  mem_init_print_info(NULL);
  kmalloc_ok = 1;
}