static int __btrfs_qgroup_release_data(struct inode *inode, struct extent_changeset *reserved, u64 start, u64 len, int free)
{
  struct extent_changeset changeset;
  int trace_op = QGROUP_RELEASE;
  int ret;

  WARN_ON(!free && reserved);
  if (free && reserved)
    return qgroup_free_reserved_data(inode, reserved, start, len);
  extent_changeset_init(&changeset);
  ret = clear_record_extent_bits(&BTRFS_I(inode)->io_tree, start, start + len - 1, EXTENT_QGROUP_RESERVED, &changeset);
  if (ret < 0)
    goto out;
  if (free)
    trace_op = QGROUP_FREE;
  trace_btrfs_qgroup_release_data(inode, start, len, changeset.bytes_changed, trace_op);
  if (free)
    btrfs_qgroup_free_refroot(BTRFS_I(inode)->root->fs_info, BTRFS_I(inode)->root->root_key.objectid, changeset.bytes_changed, BTRFS_QGROUP_RSV_DATA);
  ret = changeset.bytes_changed;
out:
  extent_changeset_release(&changeset);
  return ret;
}