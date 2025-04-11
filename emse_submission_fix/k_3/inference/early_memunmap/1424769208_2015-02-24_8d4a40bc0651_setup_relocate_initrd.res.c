static void __init relocate_initrd(void) {
  /* Assume only end is not page aligned */
  u64 ramdisk_image = get_ramdisk_image();
  u64 ramdisk_size = get_ramdisk_size();
  u64 area_size = PAGE_ALIGN(ramdisk_size);
  unsigned long slop, clen, mapaddr;
  char *p, *q;
  /* We need to move the initrd down into directly mapped mem */
  relocated_ramdisk = memblock_find_in_range(0, PFN_PHYS(max_pfn_mapped), area_size, PAGE_SIZE);
  if (!relocated_ramdisk) panic("Cannot find place for new RAMDISK of size %lld\n", ramdisk_size);
  /* Note: this includes all the mem currently occupied by
     the initrd, we rely on that fact to keep the data intact. */
  memblock_reserve(relocated_ramdisk, area_size);
  initrd_start = relocated_ramdisk + PAGE_OFFSET;
  initrd_end = initrd_start + ramdisk_size;
  printk(KERN_INFO "Allocated new RAMDISK: [mem %#010llx-%#010llx]\n", relocated_ramdisk, relocated_ramdisk + ramdisk_size - 1);
  q = (char *)initrd_start;
  /* Copy the initrd */
  while (ramdisk_size) {
    slop = ramdisk_image & ~PAGE_MASK;
    clen = ramdisk_size;
    if (clen > MAX_MAP_CHUNK - slop) clen = MAX_MAP_CHUNK - slop;
    mapaddr = ramdisk_image & PAGE_MASK;
    p = early_memremap(mapaddr, clen + slop);
    memcpy(q, p + slop, clen);
    early_memunmap(p, clen + slop);
    q += clen;
    ramdisk_image += clen;
    ramdisk_size -= clen;
  }
  ramdisk_image = get_ramdisk_image();
  ramdisk_size = get_ramdisk_size();
  printk(KERN_INFO "Move RAMDISK from [mem %#010llx-%#010llx] to"
                   " [mem %#010llx-%#010llx]\n",
         ramdisk_image, ramdisk_image + ramdisk_size - 1, relocated_ramdisk, relocated_ramdisk + ramdisk_size - 1);
}