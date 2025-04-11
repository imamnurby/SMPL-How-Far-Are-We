int btrfs_check_shared(struct btrfs_root *root, u64 inum, u64 bytenr)
{
  struct btrfs_fs_info *fs_info = root->fs_info;
  struct btrfs_trans_handle *trans;
  struct ulist *tmp = NULL;
  struct ulist *roots = NULL;
  struct ulist_iterator uiter;
  struct ulist_node *node;
  struct seq_list elem = SEQ_LIST_INIT(elem);
  int ret = 0;
  struct share_check shared = {
      .root_objectid = root->root_key.root_key.objectid,
      .inum = inum,
      .share_count = 0,
  };
  tmp = ulist_alloc(GFP_NOFS);
  roots = ulist_alloc(GFP_NOFS);
  if (!tmp || !roots)
    {
      ulist_free(tmp);
      ulist_free(roots);
      return -ENOMEM;
    }
  trans = btrfs_join_transaction(root);
  if (IS_ERR(trans))
    {
      trans = NULL;
      down_read(&fs_info->commit_root_sem);
    }
  else
    {
      btrfs_get_tree_mod_seq(fs_info, &elem);
    }
  ULIST_ITER_INIT(&uiter);
  while (1)
    {
      ret = find_parent_nodes(trans, fs_info, bytenr, elem.seq, tmp, roots, NULL, &shared, false);
      if (ret == BACKREF_FOUND_SHARED)
        {

          ret = 1;
          break;
        }
      if (ret < 0 && ret != -ENOENT)
        break;
      ret = 0;
      node = ulist_next(tmp, &uiter);
      if (!node)
        break;
      bytenr = node->val;
      shared.share_count = 0;
      cond_resched();
    }
  if (trans)
    {
      btrfs_put_tree_mod_seq(fs_info, &elem);
      btrfs_end_transaction(trans);
    }
  else
    {
      up_read(&fs_info->commit_root_sem);
    }
  ulist_free(tmp);
  ulist_free(roots);
  return ret;
}