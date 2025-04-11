int __btrfs_qgroup_reserve_meta(struct btrfs_root *root, int num_bytes, enum btrfs_qgroup_rsv_type type, bool enforce)
{
  struct btrfs_fs_info *fs_info = root->fs_info;
  int ret;
  if (!test_bit(BTRFS_FS_QUOTA_ENABLED, &fs_info->flags) || !is_fstree(root->root_key.objectid) || num_bytes == 0)
    return 0;
  BUG_ON(num_bytes != round_down(num_bytes, fs_info->nodesize));
  trace_qgroup_meta_reserve(root, type, (s64)num_bytes);
  ret = qgroup_reserve(root, num_bytes, enforce, type);
  if (ret < 0)
    return ret;

  add_root_meta_rsv(root, num_bytes, type);
  return ret;
}