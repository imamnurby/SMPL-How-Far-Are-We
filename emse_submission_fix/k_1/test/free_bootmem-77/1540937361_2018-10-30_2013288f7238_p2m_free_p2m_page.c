static void __ref free_p2m_page(void *p) {
  if (unlikely(!slab_is_available())) {
    free_bootmem((unsigned long)p, PAGE_SIZE);
    return;
  }
  free_page((unsigned long)p);
}