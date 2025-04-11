static int btrfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
  struct btrfs_fs_info *fs_info = btrfs_sb(dentry->d_sb);
  struct btrfs_super_block *disk_super = fs_info->super_copy;
  struct list_head *head = &fs_info->space_info;
  struct btrfs_space_info *found;
  u64 total_used = 0;
  u64 total_free_data = 0;
  u64 total_free_meta = 0;
  int bits = dentry->d_sb->s_blocksize_bits;
  __be32 *fsid = (__be32 *)fs_info->fsid;
  unsigned factor = 1;
  struct btrfs_block_rsv *block_rsv = &fs_info->global_block_rsv;
  int ret;
  u64 thresh = 0;
  int mixed = 0;
  rcu_read_lock();
  list_for_each_entry_rcu(found, head, list)
  {
    if (found->flags & BTRFS_BLOCK_GROUP_DATA)
      {
        int i;
        total_free_data += found->disk_total - found->disk_used;
        total_free_data -= btrfs_account_ro_block_groups_free_space(found);
        for (i = 0; i < BTRFS_NR_RAID_TYPES; i++)
          {
            if (!list_empty(&found->block_groups[i]))
              factor = btrfs_bg_type_to_factor(btrfs_raid_array[i].bg_flag);
          }
      }

    if (!mixed && found->flags & BTRFS_BLOCK_GROUP_METADATA)
      {
        if (found->flags & BTRFS_BLOCK_GROUP_DATA)
          mixed = 1;
        else
          total_free_meta += found->disk_total - found->disk_used;
      }
    total_used += found->disk_used;
  }
  rcu_read_unlock();
  buf->f_blocks = div_u64(btrfs_super_total_bytes(disk_super), factor);
  buf->f_blocks >>= bits;
  buf->f_bfree = buf->f_blocks - (div_u64(total_used, factor) >> bits);

  spin_lock(&block_rsv->lock);

  if (buf->f_bfree >= block_rsv->size >> bits)
    buf->f_bfree -= block_rsv->size >> bits;
  else
    buf->f_bfree = 0;
  spin_unlock(&block_rsv->lock);
  buf->f_bavail = div_u64(total_free_data, factor);
  ret = btrfs_calc_avail_data_space(fs_info, &total_free_data);
  if (ret)
    return ret;
  buf->f_bavail += div_u64(total_free_data, factor);
  buf->f_bavail = buf->f_bavail >> bits;

  thresh = SZ_4M;
  if (!mixed && total_free_meta - thresh < block_rsv->size)
    buf->f_bavail = 0;
  buf->f_type = BTRFS_SUPER_MAGIC;
  buf->f_bsize = dentry->d_sb->s_blocksize;
  buf->f_namelen = BTRFS_NAME_LEN;

  buf->f_fsid.val[0] = be32_to_cpu(fsid[0]) ^ be32_to_cpu(fsid[2]);
  buf->f_fsid.val[1] = be32_to_cpu(fsid[1]) ^ be32_to_cpu(fsid[3]);

  buf->f_fsid.val[0] ^= BTRFS_I(d_inode(dentry))->root->root_key.objectid >> 32;
  buf->f_fsid.val[1] ^= BTRFS_I(d_inode(dentry))->root->root_key.objectid;
  return 0;
}