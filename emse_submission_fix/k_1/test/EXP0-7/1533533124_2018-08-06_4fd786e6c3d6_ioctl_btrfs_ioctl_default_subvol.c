static long btrfs_ioctl_default_subvol(struct file *file, void __user *argp)
{
	struct inode *inode = file_inode(file);
	struct btrfs_fs_info *fs_info = btrfs_sb(inode->i_sb);
	struct btrfs_root *root = BTRFS_I(inode)->root;
	struct btrfs_root *new_root;
	struct btrfs_dir_item *di;
	struct btrfs_trans_handle *trans;
	struct btrfs_path *path;
	struct btrfs_key location;
	struct btrfs_disk_key disk_key;
	u64 objectid = 0;
	u64 dir_id;
	int ret;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = mnt_want_write_file(file);
	if (ret)
		return ret;
	if (copy_from_user(&objectid, argp, sizeof(objectid))) {
		ret = -EFAULT;
		goto out;
	}
	if (!objectid)
		objectid = BTRFS_FS_TREE_OBJECTID;
	location.objectid = objectid;
	location.type = BTRFS_ROOT_ITEM_KEY;
	location.offset = (u64)-1;
	new_root = btrfs_read_fs_root_no_name(fs_info, &location);
	if (IS_ERR(new_root)) {
		ret = PTR_ERR(new_root);
		goto out;
	}
	if (!is_fstree(new_root->objectid)) {
		ret = -ENOENT;
		goto out;
	}
	path = btrfs_alloc_path();
	if (!path) {
		ret = -ENOMEM;
		goto out;
	}
	path->leave_spinning = 1;
	trans = btrfs_start_transaction(root, 1);
	if (IS_ERR(trans)) {
		btrfs_free_path(path);
		ret = PTR_ERR(trans);
		goto out;
	}
	dir_id = btrfs_super_root_dir(fs_info->super_copy);
	di = btrfs_lookup_dir_item(trans, fs_info->tree_root, path,
				   dir_id, "default", 7, 1);
	if (IS_ERR_OR_NULL(di)) {
		btrfs_free_path(path);
		btrfs_end_transaction(trans);
		btrfs_err(fs_info,
			  "Umm, you don't have the default diritem, this isn't going to work");
		ret = -ENOENT;
		goto out;
	}
	btrfs_cpu_key_to_disk(&disk_key, &new_root->root_key);
	btrfs_set_dir_item_key(path->nodes[0], di, &disk_key);
	btrfs_mark_buffer_dirty(path->nodes[0]);
	btrfs_free_path(path);
	btrfs_set_fs_incompat(fs_info, DEFAULT_SUBVOL);
	btrfs_end_transaction(trans);
out:
	mnt_drop_write_file(file);
	return ret;
}