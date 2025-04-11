static int btrfs_encode_fh(struct inode *inode, u32 *fh, int *max_len, struct inode *parent)
{
  struct btrfs_fid *fid = (struct btrfs_fid *)fh;
  int len = *max_len;
  int type;
  if (parent && (len < BTRFS_FID_SIZE_CONNECTABLE))
    {
      *max_len = BTRFS_FID_SIZE_CONNECTABLE;
      return FILEID_INVALID;
    }
  else if (len < BTRFS_FID_SIZE_NON_CONNECTABLE)
    {
      *max_len = BTRFS_FID_SIZE_NON_CONNECTABLE;
      return FILEID_INVALID;
    }
  len = BTRFS_FID_SIZE_NON_CONNECTABLE;
  type = FILEID_BTRFS_WITHOUT_PARENT;
  fid->root_key.objectid = btrfs_ino(BTRFS_I(inode));
  fid->root_objectid = BTRFS_I(inode)->root->objectid;
  fid->gen = inode->i_generation;
  if (parent)
    {
      u64 parent_root_id;
      fid->parent_objectid = BTRFS_I(parent)->location.objectid;
      fid->parent_gen = parent->i_generation;
      parent_root_id = BTRFS_I(parent)->root->objectid;
      if (parent_root_id != fid->root_objectid)
        {
          fid->parent_root_objectid = parent_root_id;
          len = BTRFS_FID_SIZE_CONNECTABLE_ROOT;
          type = FILEID_BTRFS_WITH_PARENT_ROOT;
        }
      else
        {
          len = BTRFS_FID_SIZE_CONNECTABLE;
          type = FILEID_BTRFS_WITH_PARENT;
        }
    }
  *max_len = len;
  return type;
}