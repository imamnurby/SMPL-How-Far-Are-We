static inline void free_memmap(unsigned long start_pfn, unsigned long end_pfn) {
  struct page *start_pg, *end_pg;
  unsigned long pg, pgend;
  /*
   * Convert start_pfn/end_pfn to a struct page pointer.
   */
  start_pg = pfn_to_page(start_pfn - 1) + 1;
  end_pg = pfn_to_page(end_pfn - 1) + 1;
  /*
   * Convert to physical addresses, and round start upwards and end
   * downwards.
   */
  pg = (unsigned long)PAGE_ALIGN(__pa(start_pg));
  pgend = (unsigned long)__pa(end_pg) & PAGE_MASK;
  /*
   * If there are free pages between these, free the section of the
   * memmap array.
   */
  if (pg < pgend) memblock_free(pg, pgend - pg);
}