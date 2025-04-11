@@
expression root, inode;
identifier objectid_field;
@@
- root->objectid_field
+ root->root_key.objectid_field
...
- BTRFS_I(inode)->root->objectid_field
+ BTRFS_I(inode)->root->root_key.objectid_field