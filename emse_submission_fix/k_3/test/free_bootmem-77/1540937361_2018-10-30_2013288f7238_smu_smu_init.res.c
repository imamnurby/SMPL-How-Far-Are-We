int __init smu_init(void) {
  struct device_node *np;
  const u32 *data;
  int ret = 0;
  np = of_find_node_by_type(NULL, "smu");
  if (np == NULL) return -ENODEV;
  printk(KERN_INFO "SMU: Driver %s %s\n", VERSION, AUTHOR);
  /*
   * SMU based G5s need some memory below 2Gb. Thankfully this is
   * called at a time where memblock is still available.
   */
  smu_cmdbuf_abs = memblock_alloc_base(4096, 4096, 0x80000000UL);
  if (smu_cmdbuf_abs == 0) {
    printk(KERN_ERR "SMU: Command buffer allocation failed !\n");
    ret = -EINVAL;
    goto fail_np;
  }
  smu = memblock_alloc(sizeof(struct smu_device), 0);
  spin_lock_init(&smu->lock);
  INIT_LIST_HEAD(&smu->cmd_list);
  INIT_LIST_HEAD(&smu->cmd_i2c_list);
  smu->of_node = np;
  smu->db_irq = 0;
  smu->msg_irq = 0;
  /* smu_cmdbuf_abs is in the low 2G of RAM, can be converted to a
   * 32 bits value safely
   */
  smu->cmd_buf_abs = (u32)smu_cmdbuf_abs;
  smu->cmd_buf = __va(smu_cmdbuf_abs);
  smu->db_node = of_find_node_by_name(NULL, "smu-doorbell");
  if (smu->db_node == NULL) {
    printk(KERN_ERR "SMU: Can't find doorbell GPIO !\n");
    ret = -ENXIO;
    goto fail_bootmem;
  }
  data = of_get_property(smu->db_node, "reg", NULL);
  if (data == NULL) {
    printk(KERN_ERR "SMU: Can't find doorbell GPIO address !\n");
    ret = -ENXIO;
    goto fail_db_node;
  }
  /* Current setup has one doorbell GPIO that does both doorbell
   * and ack. GPIOs are at 0x50, best would be to find that out
   * in the device-tree though.
   */
  smu->doorbell = *data;
  if (smu->doorbell < 0x50) smu->doorbell += 0x50;
  /* Now look for the smu-interrupt GPIO */
  do {
    smu->msg_node = of_find_node_by_name(NULL, "smu-interrupt");
    if (smu->msg_node == NULL) break;
    data = of_get_property(smu->msg_node, "reg", NULL);
    if (data == NULL) {
      of_node_put(smu->msg_node);
      smu->msg_node = NULL;
      break;
    }
    smu->msg = *data;
    if (smu->msg < 0x50) smu->msg += 0x50;
  } while (0);
  /* Doorbell buffer is currently hard-coded, I didn't find a proper
   * device-tree entry giving the address. Best would probably to use
   * an offset for K2 base though, but let's do it that way for now.
   */
  smu->db_buf = ioremap(0x8000860c, 0x1000);
  if (smu->db_buf == NULL) {
    printk(KERN_ERR "SMU: Can't map doorbell buffer pointer !\n");
    ret = -ENXIO;
    goto fail_msg_node;
  }
  /* U3 has an issue with NAP mode when issuing SMU commands */
  smu->broken_nap = pmac_get_uninorth_variant() < 4;
  if (smu->broken_nap) printk(KERN_INFO "SMU: using NAP mode workaround\n");
  sys_ctrler = SYS_CTRLER_SMU;
  return 0;
fail_msg_node:
  of_node_put(smu->msg_node);
fail_db_node:
  of_node_put(smu->db_node);
fail_bootmem:
  memblock_free(__pa(smu), sizeof(struct smu_device));
  smu = NULL;
fail_np:
  of_node_put(np);
  return ret;
}