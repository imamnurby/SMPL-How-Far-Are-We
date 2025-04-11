@@
expression E;
@@
- E->objectid
+ E->root_key.objectid
@@
expression E;
@@
- BTRFS_I(E)->root->objectid
+ BTRFS_I(E)->root->root_key.objectid