backref_node *build_backref_tree(struct reloc_control *rc, struct btrfs_key *node_key, int level, u64 bytenr)
{
  struct backref_cache *cache = &rc->backref_cache;
  struct btrfs_path *path1;
  struct btrfs_path *path2;
  struct extent_buffer *eb;
  struct btrfs_root *root;
  struct backref_node *cur;
  struct backref_node *upper;
  struct backref_node *lower;
  struct backref_node *node = NULL;
  struct backref_node *exist = NULL;
  struct backref_edge *edge;
  struct rb_node *rb_node;
  struct btrfs_key key;
  unsigned long end;
  unsigned long ptr;
  LIST_HEAD(list);
  LIST_HEAD(useless);
  int cowonly;
  int ret;
  int err = 0;
  bool need_check = true;
  path1 = btrfs_alloc_path();
  path2 = btrfs_alloc_path();
  if (!path1 || !path2)
    {
      err = -ENOMEM;
      goto out;
    }
  path1->reada = READA_FORWARD;
  path2->reada = READA_FORWARD;
  node = alloc_backref_node(cache);
  if (!node)
    {
      err = -ENOMEM;
      goto out;
    }
  node->bytenr = bytenr;
  node->level = level;
  node->lowest = 1;
  cur = node;
again:
  end = 0;
  ptr = 0;
  key.objectid = cur->bytenr;
  key.type = BTRFS_METADATA_ITEM_KEY;
  key.offset = (u64)-1;
  path1->search_commit_root = 1;
  path1->skip_locking = 1;
  ret = btrfs_search_slot(NULL, rc->extent_root, &key, path1, 0, 0);
  if (ret < 0)
    {
      err = ret;
      goto out;
    }
  ASSERT(ret);
  ASSERT(path1->slots[0]);
  path1->slots[0]--;
  WARN_ON(cur->checked);
  if (!list_empty(&cur->upper))
    {

      ASSERT(list_is_singular(&cur->upper));
      edge = list_entry(cur->upper.next, struct backref_edge, list[LOWER]);
      ASSERT(list_empty(&edge->list[UPPER]));
      exist = edge->node[UPPER];

      if (!exist->checked)
        list_add_tail(&edge->list[UPPER], &list);
    }
  else
    {
      exist = NULL;
    }
  while (1)
    {
      cond_resched();
      eb = path1->nodes[0];
      if (ptr >= end)
        {
          if (path1->slots[0] >= btrfs_header_nritems(eb))
            {
              ret = btrfs_next_leaf(rc->extent_root, path1);
              if (ret < 0)
                {
                  err = ret;
                  goto out;
                }
              if (ret > 0)
                break;
              eb = path1->nodes[0];
            }
          btrfs_item_key_to_cpu(eb, &key, path1->slots[0]);
          if (key.objectid != cur->bytenr)
            {
              WARN_ON(exist);
              break;
            }
          if (key.type == BTRFS_EXTENT_ITEM_KEY || key.type == BTRFS_METADATA_ITEM_KEY)
            {
              ret = find_inline_backref(eb, path1->slots[0], &ptr, &end);
              if (ret)
                goto next;
            }
        }
      if (ptr < end)
        {

          struct btrfs_extent_inline_ref *iref;
          int type;
          iref = (struct btrfs_extent_inline_ref *)ptr;
          type = btrfs_get_extent_inline_ref_type(eb, iref, BTRFS_REF_TYPE_BLOCK);
          if (type == BTRFS_REF_TYPE_INVALID)
            {
              err = -EUCLEAN;
              goto out;
            }
          key.type = type;
          key.offset = btrfs_extent_inline_ref_offset(eb, iref);
          WARN_ON(key.type != BTRFS_TREE_BLOCK_REF_KEY && key.type != BTRFS_SHARED_BLOCK_REF_KEY);
        }
      if (exist && ((key.type == BTRFS_TREE_BLOCK_REF_KEY && exist->owner == key.offset) || (key.type == BTRFS_SHARED_BLOCK_REF_KEY && exist->bytenr == key.offset)))
        {
          exist = NULL;
          goto next;
        }
      if (key.type == BTRFS_SHARED_BLOCK_REF_KEY)
        {
          if (key.objectid == key.offset)
            {

              root = find_reloc_root(rc, cur->bytenr);
              ASSERT(root);
              cur->root = root;
              break;
            }
          edge = alloc_backref_edge(cache);
          if (!edge)
            {
              err = -ENOMEM;
              goto out;
            }
          rb_node = tree_search(&cache->rb_root, key.offset);
          if (!rb_node)
            {
              upper = alloc_backref_node(cache);
              if (!upper)
                {
                  free_backref_edge(cache, edge);
                  err = -ENOMEM;
                  goto out;
                }
              upper->bytenr = key.offset;
              upper->level = cur->level + 1;

              list_add_tail(&edge->list[UPPER], &list);
            }
          else
            {
              upper = rb_entry(rb_node, struct backref_node, rb_node);
              ASSERT(upper->checked);
              INIT_LIST_HEAD(&edge->list[UPPER]);
            }
          list_add_tail(&edge->list[LOWER], &cur->upper);
          edge->node[LOWER] = cur;
          edge->node[UPPER] = upper;
          goto next;
        }
      else if (unlikely(key.type == BTRFS_EXTENT_REF_V0_KEY))
        {
          err = -EINVAL;
          btrfs_print_v0_err(rc->extent_root->fs_info);
          btrfs_handle_fs_error(rc->extent_root->fs_info, err, NULL);
          goto out;
        }
      else if (key.type != BTRFS_TREE_BLOCK_REF_KEY)
        {
          goto next;
        }

      root = read_fs_root(rc->extent_root->fs_info, key.offset);
      if (IS_ERR(root))
        {
          err = PTR_ERR(root);
          goto out;
        }
      if (!test_bit(BTRFS_ROOT_REF_COWS, &root->state))
        cur->cowonly = 1;
      if (btrfs_root_level(&root->root_item) == cur->level)
        {

          ASSERT(btrfs_root_bytenr(&root->root_item) == cur->bytenr);
          if (should_ignore_root(root))
            list_add(&cur->list, &useless);
          else
            cur->root = root;
          break;
        }
      level = cur->level + 1;

      path2->search_commit_root = 1;
      path2->skip_locking = 1;
      path2->lowest_level = level;
      ret = btrfs_search_slot(NULL, root, node_key, path2, 0, 0);
      path2->lowest_level = 0;
      if (ret < 0)
        {
          err = ret;
          goto out;
        }
      if (ret > 0 && path2->slots[level] > 0)
        path2->slots[level]--;
      eb = path2->nodes[level];
      if (btrfs_node_blockptr(eb, path2->slots[level]) != cur->bytenr)
        {
          btrfs_err(root->fs_info, "couldn't find block (%llu) (level %d) in tree (%llu) with key (%llu %u %llu)", cur->bytenr, level - 1,
                    root->root_key.objectid, node_key->root_key.objectid,
                    node_key->type, node_key->offset);
          err = -ENOENT;
          goto out;
        }
      lower = cur;
      need_check = true;
      for (; level < BTRFS_MAX_LEVEL; level++)
        {
          if (!path2->nodes[level])
            {
              ASSERT(btrfs_root_bytenr(&root->root_item) == lower->bytenr);
              if (should_ignore_root(root))
                list_add(&lower->list, &useless);
              else
                lower->root = root;
              break;
            }
          edge = alloc_backref_edge(cache);
          if (!edge)
            {
              err = -ENOMEM;
              goto out;
            }
          eb = path2->nodes[level];
          rb_node = tree_search(&cache->rb_root, eb->start);
          if (!rb_node)
            {
              upper = alloc_backref_node(cache);
              if (!upper)
                {
                  free_backref_edge(cache, edge);
                  err = -ENOMEM;
                  goto out;
                }
              upper->bytenr = eb->start;
              upper->owner = btrfs_header_owner(eb);
              upper->level = lower->level + 1;
              if (!test_bit(BTRFS_ROOT_REF_COWS, &root->state))
                upper->cowonly = 1;

              if (btrfs_block_can_be_shared(root, eb))
                upper->checked = 0;
              else
                upper->checked = 1;

              if (!upper->checked && need_check)
                {
                  need_check = false;
                  list_add_tail(&edge->list[UPPER], &list);
                }
              else
                {
                  if (upper->checked)
                    need_check = true;
                  INIT_LIST_HEAD(&edge->list[UPPER]);
                }
            }
          else
            {
              upper = rb_entry(rb_node, struct backref_node, rb_node);
              ASSERT(upper->checked);
              INIT_LIST_HEAD(&edge->list[UPPER]);
              if (!upper->owner)
                upper->owner = btrfs_header_owner(eb);
            }
          list_add_tail(&edge->list[LOWER], &lower->upper);
          edge->node[LOWER] = lower;
          edge->node[UPPER] = upper;
          if (rb_node)
            break;
          lower = upper;
          upper = NULL;
        }
      btrfs_release_path(path2);
    next:
      if (ptr < end)
        {
          ptr += btrfs_extent_inline_ref_size(key.type);
          if (ptr >= end)
            {
              WARN_ON(ptr > end);
              ptr = 0;
              end = 0;
            }
        }
      if (ptr >= end)
        path1->slots[0]++;
    }
  btrfs_release_path(path1);
  cur->checked = 1;
  WARN_ON(exist);

  if (!list_empty(&list))
    {
      edge = list_entry(list.next, struct backref_edge, list[UPPER]);
      list_del_init(&edge->list[UPPER]);
      cur = edge->node[UPPER];
      goto again;
    }

  ASSERT(node->checked);
  cowonly = node->cowonly;
  if (!cowonly)
    {
      rb_node = tree_insert(&cache->rb_root, node->bytenr, &node->rb_node);
      if (rb_node)
        backref_tree_panic(rb_node, -EEXIST, node->bytenr);
      list_add_tail(&node->lower, &cache->leaves);
    }
  list_for_each_entry(edge, &node->upper, list[LOWER]) list_add_tail(&edge->list[UPPER], &list);
  while (!list_empty(&list))
    {
      edge = list_entry(list.next, struct backref_edge, list[UPPER]);
      list_del_init(&edge->list[UPPER]);
      upper = edge->node[UPPER];
      if (upper->detached)
        {
          list_del(&edge->list[LOWER]);
          lower = edge->node[LOWER];
          free_backref_edge(cache, edge);
          if (list_empty(&lower->upper))
            list_add(&lower->list, &useless);
          continue;
        }
      if (!RB_EMPTY_NODE(&upper->rb_node))
        {
          if (upper->lowest)
            {
              list_del_init(&upper->lower);
              upper->lowest = 0;
            }
          list_add_tail(&edge->list[UPPER], &upper->lower);
          continue;
        }
      if (!upper->checked)
        {

          ASSERT(0);
          err = -EINVAL;
          goto out;
        }
      if (cowonly != upper->cowonly)
        {
          ASSERT(0);
          err = -EINVAL;
          goto out;
        }
      if (!cowonly)
        {
          rb_node = tree_insert(&cache->rb_root, upper->bytenr, &upper->rb_node);
          if (rb_node)
            backref_tree_panic(rb_node, -EEXIST, upper->bytenr);
        }
      list_add_tail(&edge->list[UPPER], &upper->lower);
      list_for_each_entry(edge, &upper->upper, list[LOWER]) list_add_tail(&edge->list[UPPER], &list);
    }

  while (!list_empty(&useless))
    {
      upper = list_entry(useless.next, struct backref_node, list);
      list_del_init(&upper->list);
      ASSERT(list_empty(&upper->upper));
      if (upper == node)
        node = NULL;
      if (upper->lowest)
        {
          list_del_init(&upper->lower);
          upper->lowest = 0;
        }
      while (!list_empty(&upper->lower))
        {
          edge = list_entry(upper->lower.next, struct backref_edge, list[UPPER]);
          list_del(&edge->list[UPPER]);
          list_del(&edge->list[LOWER]);
          lower = edge->node[LOWER];
          free_backref_edge(cache, edge);
          if (list_empty(&lower->upper))
            list_add(&lower->list, &useless);
        }
      __mark_block_processed(rc, upper);
      if (upper->level > 0)
        {
          list_add(&upper->list, &cache->detached);
          upper->detached = 1;
        }
      else
        {
          rb_erase(&upper->rb_node, &cache->rb_root);
          free_backref_node(cache, upper);
        }
    }
out:
  btrfs_free_path(path1);
  btrfs_free_path(path2);
  if (err)
    {
      while (!list_empty(&useless))
        {
          lower = list_entry(useless.next, struct backref_node, list);
          list_del_init(&lower->list);
        }
      while (!list_empty(&list))
        {
          edge = list_first_entry(&list, struct backref_edge, list[UPPER]);
          list_del(&edge->list[UPPER]);
          list_del(&edge->list[LOWER]);
          lower = edge->node[LOWER];
          upper = edge->node[UPPER];
          free_backref_edge(cache, edge);

          if (list_empty(&lower->upper) && RB_EMPTY_NODE(&lower->rb_node))
            list_add(&lower->list, &useless);
          if (!RB_EMPTY_NODE(&upper->rb_node))
            continue;

          list_for_each_entry(edge, &upper->upper, list[LOWER]) list_add_tail(&edge->list[UPPER], &list);
          if (list_empty(&upper->upper))
            list_add(&upper->list, &useless);
        }
      while (!list_empty(&useless))
        {
          lower = list_entry(useless.next, struct backref_node, list);
          list_del_init(&lower->list);
          if (lower == node)
            node = NULL;
          free_backref_node(cache, lower);
        }
      free_backref_node(cache, node);
      return ERR_PTR(err);
    }
  ASSERT(!node || !node->detached);
  return node;
}