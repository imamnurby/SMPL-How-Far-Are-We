static inline void free_memmap(unsigned long start_pfn, unsigned long end_pfn)
{
  struct page *start_pg, *end_pg;
  unsigned long pg, pgend;

  start_pg = pfn_to_page(start_pfn - 1) + 1;
  end_pg = pfn_to_page(end_pfn);

  pg = PAGE_ALIGN(__pa(start_pg));
  pgend = __pa(end_pg) & PAGE_MASK;

  if (pg < pgend)
    memblock_free(pg, pgend - pg);
}