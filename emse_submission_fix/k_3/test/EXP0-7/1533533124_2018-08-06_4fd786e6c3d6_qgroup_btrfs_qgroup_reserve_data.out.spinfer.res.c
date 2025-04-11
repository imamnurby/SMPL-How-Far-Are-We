int btrfs_qgroup_reserve_data(struct inode *inode, struct extent_changeset **reserved_ret, u64 start, u64 len)
{
  struct btrfs_root *root = BTRFS_I(inode)->root;
  struct ulist_node *unode;
  struct ulist_iterator uiter;
  struct extent_changeset *reserved;
  u64 orig_reserved;
  u64 to_reserve;
  int ret;
  if (!test_bit(BTRFS_FS_QUOTA_ENABLED, &root->fs_info->flags) || !is_fstree(root->objectid) || len == 0)
    return 0;

  if (WARN_ON(!reserved_ret))
    return -EINVAL;
  if (!*reserved_ret)
    {
      *reserved_ret = extent_changeset_alloc();
      if (!*reserved_ret)
        return -ENOMEM;
    }
  reserved = *reserved_ret;

  orig_reserved = reserved->bytes_changed;
  ret = set_record_extent_bits(&BTRFS_I(inode)->io_tree, start, start + len - 1, EXTENT_QGROUP_RESERVED, reserved);

  to_reserve = reserved->bytes_changed - orig_reserved;
  trace_btrfs_qgroup_reserve_data(inode, start, len, to_reserve, QGROUP_RESERVE);
  if (ret < 0)
    goto cleanup;
  ret = qgroup_reserve(root, to_reserve, true, BTRFS_QGROUP_RSV_DATA);
  if (ret < 0)
    goto cleanup;
  return ret;
cleanup:

  ULIST_ITER_INIT(&uiter);
  while ((unode = ulist_next(&reserved->range_changed, &uiter)))
    clear_extent_bit(&BTRFS_I(inode)->io_tree, unode->val, unode->aux, EXTENT_QGROUP_RESERVED, 0, 0, NULL);
  extent_changeset_release(reserved);
  return ret;
}