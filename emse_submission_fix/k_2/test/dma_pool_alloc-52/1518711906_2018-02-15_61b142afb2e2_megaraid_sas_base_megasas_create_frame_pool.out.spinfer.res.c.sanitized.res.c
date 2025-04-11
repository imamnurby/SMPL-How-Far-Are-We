static int megasas_create_frame_pool(struct megasas_instance *instance)
{
  int i;
  u16 max_cmd;
  u32 sge_sz;
  u32 frame_count;
  struct megasas_cmd *cmd;
  max_cmd = instance->max_mfi_cmds;

  sge_sz = (IS_DMA64) ? sizeof(struct megasas_sge64) : sizeof(struct megasas_sge32);
  if (instance->flag_ieee)
    sge_sz = sizeof(struct megasas_sge_skinny);

  frame_count = (instance->adapter_type == MFI_SERIES) ? (15 + 1) : (3 + 1);
  instance->mfi_frame_size = MEGAMFI_FRAME_SIZE * frame_count;

  instance->frame_dma_pool = dma_pool_create("megasas frame pool", &instance->pdev->dev, instance->mfi_frame_size, 256, 0);
  if (!instance->frame_dma_pool)
    {
      dev_printk(KERN_DEBUG, &instance->pdev->dev, "failed to setup frame pool\n");
      return -ENOMEM;
    }
  instance->sense_dma_pool = dma_pool_create("megasas sense pool", &instance->pdev->dev, 128, 4, 0);
  if (!instance->sense_dma_pool)
    {
      dev_printk(KERN_DEBUG, &instance->pdev->dev, "failed to setup sense pool\n");
      dma_pool_destroy(instance->frame_dma_pool);
      instance->frame_dma_pool = NULL;
      return -ENOMEM;
    }

  for (i = 0; i < max_cmd; i++)
    {
      cmd = instance->cmd_list[i];
      cmd->frame = dma_pool_zalloc(instance->frame_dma_pool, GFP_KERNEL, &cmd->frame_phys_addr);
      cmd->sense = dma_pool_alloc(instance->sense_dma_pool, GFP_KERNEL, &cmd->sense_phys_addr);

      if (!cmd->frame || !cmd->sense)
        {
          dev_printk(KERN_DEBUG, &instance->pdev->dev, "dma_pool_alloc failed\n");
          megasas_teardown_frame_pool(instance);
          return -ENOMEM;
        }
      cmd->frame->io.context = cpu_to_le32(cmd->index);
      cmd->frame->io.pad_0 = 0;
      if ((instance->adapter_type == MFI_SERIES) && reset_devices)
        cmd->frame->hdr.cmd = MFI_CMD_INVALID;
    }
  return 0;
}