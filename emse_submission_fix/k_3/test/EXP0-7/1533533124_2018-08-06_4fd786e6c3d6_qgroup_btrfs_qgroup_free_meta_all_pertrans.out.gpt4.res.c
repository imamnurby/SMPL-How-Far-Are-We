void btrfs_qgroup_free_meta_all_pertrans(struct btrfs_root *root)
{
  struct btrfs_fs_info *fs_info = root->fs_info;
  if (!test_bit(BTRFS_FS_QUOTA_ENABLED, &fs_info->flags) || !is_fstree(root->root_key.objectid))
    return;

  trace_qgroup_meta_free_all_pertrans(root);

  btrfs_qgroup_free_refroot(fs_info, root->root_key.objectid, (u64)-1,
                            BTRFS_QGROUP_RSV_META_PERTRANS);
}