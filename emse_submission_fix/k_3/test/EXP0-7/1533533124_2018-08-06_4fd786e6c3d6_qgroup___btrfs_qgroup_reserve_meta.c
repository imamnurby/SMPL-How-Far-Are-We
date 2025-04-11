int __btrfs_qgroup_reserve_meta(struct btrfs_root *root, int num_bytes,
				enum btrfs_qgroup_rsv_type type, bool enforce)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	int ret;
	if (!test_bit(BTRFS_FS_QUOTA_ENABLED, &fs_info->flags) ||
	    !is_fstree(root->objectid) || num_bytes == 0)
		return 0;
	BUG_ON(num_bytes != round_down(num_bytes, fs_info->nodesize));
	trace_qgroup_meta_reserve(root, type, (s64)num_bytes);
	ret = qgroup_reserve(root, num_bytes, enforce, type);
	if (ret < 0)
		return ret;
	/*
	 * Record what we have reserved into root.
	 *
	 * To avoid quota disabled->enabled underflow.
	 * In that case, we may try to free space we haven't reserved
	 * (since quota was disabled), so record what we reserved into root.
	 * And ensure later release won't underflow this number.
	 */
	add_root_meta_rsv(root, num_bytes, type);
	return ret;
}