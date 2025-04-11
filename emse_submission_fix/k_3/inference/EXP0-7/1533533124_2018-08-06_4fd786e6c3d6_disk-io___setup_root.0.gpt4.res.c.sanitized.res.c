static void __setup_root(struct btrfs_root *root, struct btrfs_fs_info *fs_info, u64 objectid)
{
  bool dummy = test_bit(BTRFS_FS_STATE_DUMMY_FS_INFO, &fs_info->fs_state);
  root->root_key.node = NULL;
  root->root_key.commit_root = NULL;
  root->root_key.state = 0;
  root->root_key.orphan_cleanup_state = 0;
  root->root_key.objectid = objectid;
  root->root_key.last_trans = 0;
  root->root_key.highest_objectid = 0;
  root->root_key.nr_delalloc_inodes = 0;
  root->root_key.nr_ordered_extents = 0;
  root->root_key.inode_tree = RB_ROOT;
  INIT_RADIX_TREE(&root->root_key.delayed_nodes_tree, GFP_ATOMIC);
  root->root_key.block_rsv = NULL;
  INIT_LIST_HEAD(&root->root_key.dirty_list);
  INIT_LIST_HEAD(&root->root_key.root_list);
  INIT_LIST_HEAD(&root->root_key.delalloc_inodes);
  INIT_LIST_HEAD(&root->root_key.delalloc_root);
  INIT_LIST_HEAD(&root->root_key.ordered_extents);
  INIT_LIST_HEAD(&root->root_key.ordered_root);
  INIT_LIST_HEAD(&root->root_key.logged_list[0]);
  INIT_LIST_HEAD(&root->root_key.logged_list[1]);
  spin_lock_init(&root->root_key.inode_lock);
  spin_lock_init(&root->root_key.delalloc_lock);
  spin_lock_init(&root->root_key.ordered_extent_lock);
  spin_lock_init(&root->root_key.accounting_lock);
  spin_lock_init(&root->root_key.log_extents_lock[0]);
  spin_lock_init(&root->root_key.log_extents_lock[1]);
  spin_lock_init(&root->root_key.qgroup_meta_rsv_lock);
  mutex_init(&root->root_key.objectid_mutex);
  mutex_init(&root->root_key.log_mutex);
  mutex_init(&root->root_key.ordered_extent_mutex);
  mutex_init(&root->root_key.delalloc_mutex);
  init_waitqueue_head(&root->root_key.log_writer_wait);
  init_waitqueue_head(&root->root_key.log_commit_wait[0]);
  init_waitqueue_head(&root->root_key.log_commit_wait[1]);
  INIT_LIST_HEAD(&root->root_key.log_ctxs[0]);
  INIT_LIST_HEAD(&root->root_key.log_ctxs[1]);
  atomic_set(&root->root_key.log_commit[0], 0);
  atomic_set(&root->root_key.log_commit[1], 0);
  atomic_set(&root->root_key.log_writers, 0);
  atomic_set(&root->root_key.log_batch, 0);
  refcount_set(&root->root_key.refs, 1);
  atomic_set(&root->root_key.will_be_snapshotted, 0);
  atomic_set(&root->root_key.snapshot_force_cow, 0);
  root->root_key.log_transid = 0;
  root->root_key.log_transid_committed = -1;
  root->root_key.last_log_commit = 0;
  if (!dummy)
    extent_io_tree_init(&root->root_key.dirty_log_pages, NULL);
  memset(&root->root_key.root_key, 0, sizeof(root->root_key.root_key));
  memset(&root->root_key.root_item, 0, sizeof(root->root_key.root_item));
  memset(&root->root_key.defrag_progress, 0, sizeof(root->root_key.defrag_progress));
  if (!dummy)
    root->root_key.defrag_trans_start = fs_info->generation;
  else
    root->root_key.defrag_trans_start = 0;
  root->root_key.root_key.objectid = objectid;
  root->root_key.anon_dev = 0;
  spin_lock_init(&root->root_key.root_item_lock);
}