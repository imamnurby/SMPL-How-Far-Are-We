static int iterate_inode_refs(u64 inum, struct btrfs_root *fs_root, struct btrfs_path *path, iterate_irefs_t *iterate, void *ctx)
{
  int ret = 0;
  int slot;
  u32 cur;
  u32 len;
  u32 name_len;
  u64 parent = 0;
  int found = 0;
  struct extent_buffer *eb;
  struct btrfs_item *item;
  struct btrfs_inode_ref *iref;
  struct btrfs_key found_key;
  while (!ret)
    {
      ret = btrfs_find_item(fs_root, path, inum, parent ? parent + 1 : 0, BTRFS_INODE_REF_KEY, &found_key);
      if (ret < 0)
        break;
      if (ret)
        {
          ret = found ? 0 : -ENOENT;
          break;
        }
      ++found;
      parent = found_key.offset;
      slot = path->slots[0];
      eb = btrfs_clone_extent_buffer(path->nodes[0]);
      if (!eb)
        {
          ret = -ENOMEM;
          break;
        }
      extent_buffer_get(eb);
      btrfs_tree_read_lock(eb);
      btrfs_set_lock_blocking_rw(eb, BTRFS_READ_LOCK);
      btrfs_release_path(path);
      item = btrfs_item_nr(slot);
      iref = btrfs_item_ptr(eb, slot, struct btrfs_inode_ref);
      for (cur = 0; cur < btrfs_item_size(eb, item); cur += len)
        {
          name_len = btrfs_inode_ref_name_len(eb, iref);

          btrfs_debug(fs_root->fs_info, "following ref at offset %u for inode %llu in tree %llu", cur, found_key.objectid, fs_root->objectid);
          ret = iterate(parent, name_len, (unsigned long)(iref + 1), eb, ctx);
          if (ret)
            break;
          len = sizeof(*iref) + name_len;
          iref = (struct btrfs_inode_ref *)((char *)iref + len);
        }
      btrfs_tree_read_unlock_blocking(eb);
      free_extent_buffer(eb);
    }
  btrfs_release_path(path);
  return ret;
}