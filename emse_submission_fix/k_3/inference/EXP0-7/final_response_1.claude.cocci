@@
expression E;
@@
(
- btrfs_qgroup_free_refroot(E->root->fs_info, E->root->objectid,
+ btrfs_qgroup_free_refroot(E->root->fs_info, E->root->root_key.objectid,
  ...)
|
- btrfs_debug(E, "cleaner removing %llu", root->objectid);
+ btrfs_debug(E, "cleaner removing %llu", root->root_key.objectid);
)
@@
identifier r;
expression E;
@@
(
- r->objectid
+ r->root_key.objectid
|
- r.objectid
+ r.root_key.objectid
)
@@
identifier i, r;
expression E;
@@
(
struct share_check E = {
-  .root_objectid = r->objectid,
+  .root_objectid = r->root_key.objectid,
   ...
};
|
- E->root_objectid = r->objectid;
+ E->root_objectid = r->root_key.objectid;
|
- E.root_objectid = r->objectid;
+ E.root_objectid = r->root_key.objectid;
)
@@
identifier r;
expression E;
@@
- r->objectid = E;
+ r->root_key.objectid = E;