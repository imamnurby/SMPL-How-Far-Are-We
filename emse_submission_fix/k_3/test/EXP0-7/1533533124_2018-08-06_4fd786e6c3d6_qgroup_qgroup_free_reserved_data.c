static int qgroup_free_reserved_data(struct inode *inode,
			struct extent_changeset *reserved, u64 start, u64 len)
{
	struct btrfs_root *root = BTRFS_I(inode)->root;
	struct ulist_node *unode;
	struct ulist_iterator uiter;
	struct extent_changeset changeset;
	int freed = 0;
	int ret;
	extent_changeset_init(&changeset);
	len = round_up(start + len, root->fs_info->sectorsize);
	start = round_down(start, root->fs_info->sectorsize);
	ULIST_ITER_INIT(&uiter);
	while ((unode = ulist_next(&reserved->range_changed, &uiter))) {
		u64 range_start = unode->val;
		/* unode->aux is the inclusive end */
		u64 range_len = unode->aux - range_start + 1;
		u64 free_start;
		u64 free_len;
		extent_changeset_release(&changeset);
		/* Only free range in range [start, start + len) */
		if (range_start >= start + len ||
		    range_start + range_len <= start)
			continue;
		free_start = max(range_start, start);
		free_len = min(start + len, range_start + range_len) -
			   free_start;
		/*
		 * TODO: To also modify reserved->ranges_reserved to reflect
		 * the modification.
		 *
		 * However as long as we free qgroup reserved according to
		 * EXTENT_QGROUP_RESERVED, we won't double free.
		 * So not need to rush.
		 */
		ret = clear_record_extent_bits(&BTRFS_I(inode)->io_failure_tree,
				free_start, free_start + free_len - 1,
				EXTENT_QGROUP_RESERVED, &changeset);
		if (ret < 0)
			goto out;
		freed += changeset.bytes_changed;
	}
	btrfs_qgroup_free_refroot(root->fs_info, root->objectid, freed,
				  BTRFS_QGROUP_RSV_DATA);
	ret = freed;
out:
	extent_changeset_release(&changeset);
	return ret;
}