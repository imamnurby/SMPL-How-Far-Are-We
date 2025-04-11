int
nouveau_bo_new(struct nouveau_cli *cli, u64 size, int align,
	       uint32_t flags, uint32_t tile_mode, uint32_t tile_flags,
	       struct sg_table *sg, struct reservation_object *robj,
	       struct nouveau_bo **pnvbo)
{
	struct nouveau_drm *drm = cli->drm;
	struct nouveau_bo *nvbo;
	struct nvif_mmu *mmu = &cli->mmu;
	struct nvif_vmm *vmm = &cli->vmm.vmm;
	size_t acc_size;
	int type = ttm_bo_type_device;
	int ret, i, pi = -1;
	if (!size) {
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
	/* This is confusing, and doesn't actually mean we want an uncached
	 * mapping, but is what NOUVEAU_GEM_DOMAIN_COHERENT gets translated
	 * into in nouveau_gem_new().
	 */
	if (flags & TTM_PL_FLAG_UNCACHED) {
		/* Determine if we can get a cache-coherent map, forcing
		 * uncached mapping if we can't.
		 */
		if (!nouveau_drm_use_coherent_gpu_mapping(drm))
			nvbo->force_coherent = true;
	}
	if (cli->device.info.family >= NV_DEVICE_INFO_V0_FERMI) {
		nvbo->kind = (tile_flags & 0x0000ff00) >> 8;
		if (!nvif_mmu_kind_valid(mmu, nvbo->kind)) {
			kfree(nvbo);
			return -EINVAL;
		}
		nvbo->comp = mmu->kind[nvbo->kind] != nvbo->kind;
	} else
	if (cli->device.info.family >= NV_DEVICE_INFO_V0_TESLA) {
		nvbo->kind = (tile_flags & 0x00007f00) >> 8;
		nvbo->comp = (tile_flags & 0x00030000) >> 16;
		if (!nvif_mmu_kind_valid(mmu, nvbo->kind)) {
			kfree(nvbo);
			return -EINVAL;
		}
	} else {
		nvbo->zeta = (tile_flags & 0x00000007);
	}
	nvbo->mode = tile_mode;
	nvbo->contig = !(tile_flags & NOUVEAU_GEM_TILE_NONCONTIG);
	/* Determine the desirable target GPU page size for the buffer. */
	for (i = 0; i < vmm->page_nr; i++) {
		/* Because we cannot currently allow VMM maps to fail
		 * during buffer migration, we need to determine page
		 * size for the buffer up-front, and pre-allocate its
		 * page tables.
		 *
		 * Skip page sizes that can't support needed domains.
		 */
		if (cli->device.info.family > NV_DEVICE_INFO_V0_CURIE &&
		    (flags & TTM_PL_FLAG_VRAM) && !vmm->page[i].vram)
			continue;
		if ((flags & TTM_PL_FLAG_TT) &&
		    (!vmm->page[i].host || vmm->page[i].shift > PAGE_SHIFT))
			continue;
		/* Select this page size if it's the first that supports
		 * the potential memory domains, or when it's compatible
		 * with the requested compression settings.
		 */
		if (pi < 0 || !nvbo->comp || vmm->page[i].comp)
			pi = i;
		/* Stop once the buffer is larger than the current page size. */
		if (size >= 1ULL << vmm->page[i].shift)
			break;
	}
	if (WARN_ON(pi < 0))
		return -EINVAL;
	/* Disable compression if suitable settings couldn't be found. */
	if (nvbo->comp && !vmm->page[pi].comp) {
		if (mmu->object.oclass >= NVIF_CLASS_MMU_GF100)
			nvbo->kind = mmu->kind[nvbo->kind];
		nvbo->comp = 0;
	}
	nvbo->page = vmm->page[pi].shift;
	nouveau_bo_fixup_align(nvbo, flags, &align, &size);
	nvbo->bo.mem.num_pages = size >> PAGE_SHIFT;
	nouveau_bo_placement_set(nvbo, flags, 0);
	acc_size = ttm_bo_dma_acc_size(&drm->ttm.bdev, size,
				       sizeof(struct nouveau_bo));
	ret = ttm_bo_init(&drm->ttm.bdev, &nvbo->bo, size,
			  type, &nvbo->placement,
			  align >> PAGE_SHIFT, false, acc_size, sg,
			  robj, nouveau_bo_del_ttm);
	if (ret) {
		/* ttm will call nouveau_bo_del_ttm if it fails.. */
		return ret;
	}
	*pnvbo = nvbo;
	return 0;
}