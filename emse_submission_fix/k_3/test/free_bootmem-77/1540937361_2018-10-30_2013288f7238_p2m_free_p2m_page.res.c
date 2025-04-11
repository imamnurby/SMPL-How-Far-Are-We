static void __ref free_p2m_page(void *p) {
  if (unlikely(!slab_is_available())) {
    memblock_free((unsigned long)p, PAGE_SIZE);
    return;
  }
  free_page((unsigned long)p);
}