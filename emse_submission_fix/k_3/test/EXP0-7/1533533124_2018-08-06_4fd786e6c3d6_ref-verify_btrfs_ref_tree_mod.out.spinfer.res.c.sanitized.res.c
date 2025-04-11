int btrfs_ref_tree_mod(struct btrfs_root *root, u64 bytenr, u64 num_bytes, u64 parent, u64 ref_root, u64 owner, u64 offset, int action)
{
  struct btrfs_fs_info *fs_info = root->fs_info;
  struct ref_entry *ref = NULL, *exist;
  struct ref_action *ra = NULL;
  struct block_entry *be = NULL;
  struct root_entry *re = NULL;
  int ret = 0;
  bool metadata = owner < BTRFS_FIRST_FREE_OBJECTID;
  if (!btrfs_test_opt(root->fs_info, REF_VERIFY))
    return 0;
  ref = kzalloc(sizeof(struct ref_entry), GFP_NOFS);
  ra = kmalloc(sizeof(struct ref_action), GFP_NOFS);
  if (!ra || !ref)
    {
      kfree(ref);
      kfree(ra);
      ret = -ENOMEM;
      goto out;
    }
  if (parent)
    {
      ref->parent = parent;
    }
  else
    {
      ref->root_objectid = ref_root;
      ref->owner = owner;
      ref->offset = offset;
    }
  ref->num_refs = (action == BTRFS_DROP_DELAYED_REF) ? -1 : 1;
  memcpy(&ra->ref, ref, sizeof(struct ref_entry));

  ra->ref.owner = owner;
  ra->ref.offset = offset;
  ra->ref.root_objectid = ref_root;
  __save_stack_trace(ra);
  INIT_LIST_HEAD(&ra->list);
  ra->action = action;
  ra->root = root->objectid;

  ret = -EINVAL;
  if (action == BTRFS_ADD_DELAYED_EXTENT)
    {

      be = add_block_entry(root->fs_info, bytenr, num_bytes, ref_root);
      if (IS_ERR(be))
        {
          kfree(ra);
          ret = PTR_ERR(be);
          goto out;
        }
      be->num_refs++;
      if (metadata)
        be->metadata = 1;
      if (be->num_refs != 1)
        {
          btrfs_err(fs_info, "re-allocated a block that still has references to it!");
          dump_block_entry(fs_info, be);
          dump_ref_action(fs_info, ra);
          goto out_unlock;
        }
      while (!list_empty(&be->actions))
        {
          struct ref_action *tmp;
          tmp = list_first_entry(&be->actions, struct ref_action, list);
          list_del(&tmp->list);
          kfree(tmp);
        }
    }
  else
    {
      struct root_entry *tmp;
      if (!parent)
        {
          re = kmalloc(sizeof(struct root_entry), GFP_NOFS);
          if (!re)
            {
              kfree(ref);
              kfree(ra);
              ret = -ENOMEM;
              goto out;
            }

          ref_root = root->objectid;
          re->root_objectid = root->objectid;
          re->num_refs = 0;
        }
      spin_lock(&root->fs_info->ref_verify_lock);
      be = lookup_block_entry(&root->fs_info->block_tree, bytenr);
      if (!be)
        {
          btrfs_err(fs_info, "trying to do action %d to bytenr %llu num_bytes %llu but there is no existing entry!", action, (unsigned long long)bytenr, (unsigned long long)num_bytes);
          dump_ref_action(fs_info, ra);
          kfree(ref);
          kfree(ra);
          goto out_unlock;
        }
      if (!parent)
        {
          tmp = insert_root_entry(&be->roots, re);
          if (tmp)
            {
              kfree(re);
              re = tmp;
            }
        }
    }
  exist = insert_ref_entry(&be->refs, ref);
  if (exist)
    {
      if (action == BTRFS_DROP_DELAYED_REF)
        {
          if (exist->num_refs == 0)
            {
              btrfs_err(fs_info, "dropping a ref for a existing root that doesn't have a ref on the block");
              dump_block_entry(fs_info, be);
              dump_ref_action(fs_info, ra);
              kfree(ra);
              goto out_unlock;
            }
          exist->num_refs--;
          if (exist->num_refs == 0)
            {
              rb_erase(&exist->node, &be->refs);
              kfree(exist);
            }
        }
      else if (!be->metadata)
        {
          exist->num_refs++;
        }
      else
        {
          btrfs_err(fs_info, "attempting to add another ref for an existing ref on a tree block");
          dump_block_entry(fs_info, be);
          dump_ref_action(fs_info, ra);
          kfree(ra);
          goto out_unlock;
        }
      kfree(ref);
    }
  else
    {
      if (action == BTRFS_DROP_DELAYED_REF)
        {
          btrfs_err(fs_info, "dropping a ref for a root that doesn't have a ref on the block");
          dump_block_entry(fs_info, be);
          dump_ref_action(fs_info, ra);
          kfree(ra);
          goto out_unlock;
        }
    }
  if (!parent && !re)
    {
      re = lookup_root_entry(&be->roots, ref_root);
      if (!re)
        {

          btrfs_err(fs_info, "failed to find root %llu for %llu", root->objectid, be->bytenr);
          dump_block_entry(fs_info, be);
          dump_ref_action(fs_info, ra);
          kfree(ra);
          goto out_unlock;
        }
    }
  if (action == BTRFS_DROP_DELAYED_REF)
    {
      if (re)
        re->num_refs--;
      be->num_refs--;
    }
  else if (action == BTRFS_ADD_DELAYED_REF)
    {
      be->num_refs++;
      if (re)
        re->num_refs++;
    }
  list_add_tail(&ra->list, &be->actions);
  ret = 0;
out_unlock:
  spin_unlock(&root->fs_info->ref_verify_lock);
out:
  if (ret)
    btrfs_clear_opt(fs_info->mount_opt, REF_VERIFY);
  return ret;
}