void btrfs_qgroup_convert_reserved_meta(struct btrfs_root *root, int num_bytes)
{
  struct btrfs_fs_info *fs_info = root->fs_info;
  if (!test_bit(BTRFS_FS_QUOTA_ENABLED, &fs_info->flags) || !is_fstree(root->objectid))
    return;

  num_bytes = sub_root_meta_rsv(root, num_bytes, BTRFS_QGROUP_RSV_META_PREALLOC);
  trace_qgroup_meta_convert(root, num_bytes);
  qgroup_convert_meta(fs_info, root->objectid, num_bytes);
}