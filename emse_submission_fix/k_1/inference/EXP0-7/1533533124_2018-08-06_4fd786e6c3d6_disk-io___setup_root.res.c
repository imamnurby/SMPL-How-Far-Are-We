static void __setup_root(struct btrfs_root *root, struct btrfs_fs_info *fs_info,
			 u64 objectid)
{
	bool dummy = test_bit(BTRFS_FS_STATE_DUMMY_FS_INFO, &fs_info->fs_state);
	root->node = NULL;
	root->commit_root = NULL;
	root->state = 0;
	root->orphan_cleanup_state = 0;
	root->root_key.objectid = objectid;
	root->last_trans = 0;
	root->highest_objectid = 0;
	root->nr_delalloc_inodes = 0;
	root->nr_ordered_extents = 0;
	root->inode_tree = RB_ROOT;
	INIT_RADIX_TREE(&root->delayed_nodes_tree, GFP_ATOMIC);
	root->block_rsv = NULL;
	INIT_LIST_HEAD(&root->dirty_list);
	INIT_LIST_HEAD(&root->root_list);
	INIT_LIST_HEAD(&root->delalloc_inodes);
	INIT_LIST_HEAD(&root->delalloc_root);
	INIT_LIST_HEAD(&root->ordered_extents);
	INIT_LIST_HEAD(&root->ordered_root);
	INIT_LIST_HEAD(&root->logged_list[0]);
	INIT_LIST_HEAD(&root->logged_list[1]);
	spin_lock_init(&root->inode_lock);
	spin_lock_init(&root->delalloc_lock);
	spin_lock_init(&root->ordered_extent_lock);
	spin_lock_init(&root->accounting_lock);
	spin_lock_init(&root->log_extents_lock[0]);
	spin_lock_init(&root->log_extents_lock[1]);
	spin_lock_init(&root->qgroup_meta_rsv_lock);
	mutex_init(&root->objectid_mutex);
	mutex_init(&root->log_mutex);
	mutex_init(&root->ordered_extent_mutex);
	mutex_init(&root->delalloc_mutex);
	init_waitqueue_head(&root->log_writer_wait);
	init_waitqueue_head(&root->log_commit_wait[0]);
	init_waitqueue_head(&root->log_commit_wait[1]);
	INIT_LIST_HEAD(&root->log_ctxs[0]);
	INIT_LIST_HEAD(&root->log_ctxs[1]);
	atomic_set(&root->log_commit[0], 0);
	atomic_set(&root->log_commit[1], 0);
	atomic_set(&root->log_writers, 0);
	atomic_set(&root->log_batch, 0);
	refcount_set(&root->refs, 1);
	atomic_set(&root->will_be_snapshotted, 0);
	atomic_set(&root->snapshot_force_cow, 0);
	root->log_transid = 0;
	root->log_transid_committed = -1;
	root->last_log_commit = 0;
	if (!dummy)
		extent_io_tree_init(&root->dirty_log_pages, NULL);
	memset(&root->root_key, 0, sizeof(root->root_key));
	memset(&root->root_item, 0, sizeof(root->root_item));
	memset(&root->defrag_progress, 0, sizeof(root->defrag_progress));
	if (!dummy)
		root->defrag_trans_start = fs_info->generation;
	else
		root->defrag_trans_start = 0;
	root->root_key.objectid = objectid;
	root->anon_dev = 0;
	spin_lock_init(&root->root_item_lock);
}