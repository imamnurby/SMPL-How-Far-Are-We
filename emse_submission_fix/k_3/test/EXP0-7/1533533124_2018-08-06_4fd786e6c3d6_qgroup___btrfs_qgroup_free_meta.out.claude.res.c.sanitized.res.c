void __btrfs_qgroup_free_meta(struct btrfs_root *root, int num_bytes, enum btrfs_qgroup_rsv_type type)
{
  struct btrfs_fs_info *fs_info = root->fs_info;
  if (!test_bit(BTRFS_FS_QUOTA_ENABLED, &fs_info->flags) || !is_fstree(root->root_key.objectid))
    return;

  num_bytes = sub_root_meta_rsv(root, num_bytes, type);
  BUG_ON(num_bytes != round_down(num_bytes, fs_info->nodesize));
  trace_qgroup_meta_reserve(root, type, -(s64)num_bytes);
  btrfs_qgroup_free_refroot(fs_info, root->root_key.objectid, num_bytes, type);
}