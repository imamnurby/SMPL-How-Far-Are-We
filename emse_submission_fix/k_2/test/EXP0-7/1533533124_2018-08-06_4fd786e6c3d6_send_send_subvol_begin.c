static int send_subvol_begin(struct send_ctx *sctx)
{
	int ret;
	struct btrfs_root *send_root = sctx->send_root;
	struct btrfs_root *parent_root = sctx->parent_root;
	struct btrfs_path *path;
	struct btrfs_key key;
	struct btrfs_root_ref *ref;
	struct extent_buffer *leaf;
	char *name = NULL;
	int namelen;
	path = btrfs_alloc_path();
	if (!path)
		return -ENOMEM;
	name = kmalloc(BTRFS_PATH_NAME_MAX, GFP_KERNEL);
	if (!name) {
		btrfs_free_path(path);
		return -ENOMEM;
	}
	key.objectid = send_root->objectid;
	key.type = BTRFS_ROOT_BACKREF_KEY;
	key.offset = 0;
	ret = btrfs_search_slot_for_read(send_root->fs_info->tree_root,
				&key, path, 1, 0);
	if (ret < 0)
		goto out;
	if (ret) {
		ret = -ENOENT;
		goto out;
	}
	leaf = path->nodes[0];
	btrfs_item_key_to_cpu(leaf, &key, path->slots[0]);
	if (key.type != BTRFS_ROOT_BACKREF_KEY ||
	    key.objectid != send_root->objectid) {
		ret = -ENOENT;
		goto out;
	}
	ref = btrfs_item_ptr(leaf, path->slots[0], struct btrfs_root_ref);
	namelen = btrfs_root_ref_name_len(leaf, ref);
	read_extent_buffer(leaf, name, (unsigned long)(ref + 1), namelen);
	btrfs_release_path(path);
	if (parent_root) {
		ret = begin_cmd(sctx, BTRFS_SEND_C_SNAPSHOT);
		if (ret < 0)
			goto out;
	} else {
		ret = begin_cmd(sctx, BTRFS_SEND_C_SUBVOL);
		if (ret < 0)
			goto out;
	}
	TLV_PUT_STRING(sctx, BTRFS_SEND_A_PATH, name, namelen);
	if (!btrfs_is_empty_uuid(sctx->send_root->root_item.received_uuid))
		TLV_PUT_UUID(sctx, BTRFS_SEND_A_UUID,
			    sctx->send_root->root_item.received_uuid);
	else
		TLV_PUT_UUID(sctx, BTRFS_SEND_A_UUID,
			    sctx->send_root->root_item.uuid);
	TLV_PUT_U64(sctx, BTRFS_SEND_A_CTRANSID,
		    le64_to_cpu(sctx->send_root->root_item.ctransid));
	if (parent_root) {
		if (!btrfs_is_empty_uuid(parent_root->root_item.received_uuid))
			TLV_PUT_UUID(sctx, BTRFS_SEND_A_CLONE_UUID,
				     parent_root->root_item.received_uuid);
		else
			TLV_PUT_UUID(sctx, BTRFS_SEND_A_CLONE_UUID,
				     parent_root->root_item.uuid);
		TLV_PUT_U64(sctx, BTRFS_SEND_A_CLONE_CTRANSID,
			    le64_to_cpu(sctx->parent_root->root_item.ctransid));
	}
	ret = send_cmd(sctx);
tlv_put_failure:
out:
	btrfs_free_path(path);
	kfree(name);
	return ret;
}