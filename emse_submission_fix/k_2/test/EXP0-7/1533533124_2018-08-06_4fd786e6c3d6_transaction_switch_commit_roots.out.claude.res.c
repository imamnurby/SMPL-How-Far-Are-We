static noinline void switch_commit_roots(struct btrfs_transaction *trans)
{
  struct btrfs_fs_info *fs_info = trans->fs_info;
  struct btrfs_root *root, *tmp;
  down_write(&fs_info->commit_root_sem);
  list_for_each_entry_safe(root, tmp, &trans->switch_commits, dirty_list)
  {
    list_del_init(&root->dirty_list);
    free_extent_buffer(root->commit_root);
    root->commit_root = btrfs_root_node(root);
    if (is_fstree(root->root_key.objectid))
      btrfs_unpin_free_ino(root);
    clear_btree_io_tree(&root->dirty_log_pages);
  }

  spin_lock(&trans->dropped_roots_lock);
  while (!list_empty(&trans->dropped_roots))
    {
      root = list_first_entry(&trans->dropped_roots, struct btrfs_root, root_list);
      list_del_init(&root->root_list);
      spin_unlock(&trans->dropped_roots_lock);
      btrfs_drop_and_free_fs_root(fs_info, root);
      spin_lock(&trans->dropped_roots_lock);
    }
  spin_unlock(&trans->dropped_roots_lock);
  up_write(&fs_info->commit_root_sem);
}