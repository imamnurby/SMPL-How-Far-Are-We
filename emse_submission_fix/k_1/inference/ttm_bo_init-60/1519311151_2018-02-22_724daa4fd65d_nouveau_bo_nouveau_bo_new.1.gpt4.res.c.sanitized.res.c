int nouveau_bo_new(struct nouveau_cli *cli, u64 size, int align, uint32_t flags, uint32_t tile_mode, uint32_t tile_flags, struct sg_table *sg, struct reservation_object *robj, struct nouveau_bo **pnvbo)
{
  struct nouveau_drm *drm = cli->drm;
  struct nouveau_bo *nvbo;
  struct nvif_mmu *mmu = &cli->mmu;
  struct nvif_vmm *vmm = &cli->vmm.vmm;
  size_t acc_size;
  int type = ttm_bo_type_device;
  int ret, i, pi = -1;
  if (!size)
    {
      NV_WARN(drm, "skipped size %016llx\n", size);
      return -EINVAL;
    }
  if (sg)
    type = ttm_bo_type_sg;
  nvbo = kzalloc(sizeof(struct nouveau_bo), GFP_KERNEL);
  if (!nvbo)
    return -ENOMEM;
  INIT_LIST_HEAD(&nvbo->head);
  INIT_LIST_HEAD(&nvbo->entry);
  INIT_LIST_HEAD(&nvbo->vma_list);
  nvbo->bo.bdev = &drm->ttm.bdev;
  nvbo->cli = cli;

  if (flags & TTM_PL_FLAG_UNCACHED)
    {

      if (!nouveau_drm_use_coherent_gpu_mapping(drm))
        nvbo->force_coherent = true;
    }
  if (cli->device.info.family >= NV_DEVICE_INFO_V0_FERMI)
    {
      nvbo->kind = (tile_flags & 0x0000ff00) >> 8;
      if (!nvif_mmu_kind_valid(mmu, nvbo->kind))
        {
          kfree(nvbo);
          return -EINVAL;
        }
      nvbo->comp = mmu->kind[nvbo->kind] != nvbo->kind;
    }
  else if (cli->device.info.family >= NV_DEVICE_INFO_V0_TESLA)
    {
      nvbo->kind = (tile_flags & 0x00007f00) >> 8;
      nvbo->comp = (tile_flags & 0x00030000) >> 16;
      if (!nvif_mmu_kind_valid(mmu, nvbo->kind))
        {
          kfree(nvbo);
          return -EINVAL;
        }
    }
  else
    {
      nvbo->zeta = (tile_flags & 0x00000007);
    }
  nvbo->mode = tile_mode;
  nvbo->contig = !(tile_flags & NOUVEAU_GEM_TILE_NONCONTIG);

  for (i = 0; i < vmm->page_nr; i++)
    {

      if (cli->device.info.family > NV_DEVICE_INFO_V0_CURIE && (flags & TTM_PL_FLAG_VRAM) && !vmm->page[i].vram)
        continue;
      if ((flags & TTM_PL_FLAG_TT) && (!vmm->page[i].host || vmm->page[i].shift > PAGE_SHIFT))
        continue;

      if (pi < 0 || !nvbo->comp || vmm->page[i].comp)
        pi = i;

      if (size >= 1ULL << vmm->page[i].shift)
        break;
    }
  if (WARN_ON(pi < 0))
    return -EINVAL;

  if (nvbo->comp && !vmm->page[pi].comp)
    {
      if (mmu->object.oclass >= NVIF_CLASS_MMU_GF100)
        nvbo->kind = mmu->kind[nvbo->kind];
      nvbo->comp = 0;
    }
  nvbo->page = vmm->page[pi].shift;
  nouveau_bo_fixup_align(nvbo, flags, &align, &size);
  nvbo->bo.mem.num_pages = size >> PAGE_SHIFT;
  nouveau_bo_placement_set(nvbo, flags, 0);
  acc_size = ttm_bo_dma_acc_size(&drm->ttm.bdev, size, sizeof(struct nouveau_bo));
  ret = ttm_bo_init(&drm->ttm.bdev, &nvbo->bo, size, type, &nvbo->placement, align >> PAGE_SHIFT, false, NULL, acc_size, sg, robj, nouveau_bo_del_ttm);
  if (ret)
    {

      return ret;
    }
  *pnvbo = nvbo;
  return 0;
}