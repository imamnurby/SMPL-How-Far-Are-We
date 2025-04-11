int btrfs_clean_one_deleted_snapshot(struct btrfs_root *root)
{
  int ret;
  struct btrfs_fs_info *fs_info = root->fs_info;
  spin_lock(&fs_info->trans_lock);
  if (list_empty(&fs_info->dead_roots))
    {
      spin_unlock(&fs_info->trans_lock);
      return 0;
    }
  root = list_first_entry(&fs_info->dead_roots, struct btrfs_root, root_list);
  list_del_init(&root->root_list);
  spin_unlock(&fs_info->trans_lock);
  btrfs_debug(fs_info, "cleaner removing %llu",
              root->root_key.root_key.objectid);
  btrfs_kill_all_delayed_nodes(root);
  if (btrfs_header_backref_rev(root->node) < BTRFS_MIXED_BACKREF_REV)
    ret = btrfs_drop_snapshot(root, NULL, 0, 0);
  else
    ret = btrfs_drop_snapshot(root, NULL, 1, 0);
  return (ret < 0) ? 0 : 1;
}