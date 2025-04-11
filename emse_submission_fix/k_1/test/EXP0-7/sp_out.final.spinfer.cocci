@@
expression E0, E1;
@@
- E0->objectid = E1; 
+ E0->root_key.objectid = E1; 
// Infered from: (./EXP0-7/1533533124_2018-08-06_4fd786e6c3d6_disk-io___setup_root.{c.sanitized.c,res.c.sanitized.res.c}: __setup_root)
// Recall: 0.50, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
@@
expression E0, E1;
@@
- btrfs_qgroup_free_refroot(BTRFS_I(E0)->root->fs_info,                           BTRFS_I(E0)->root->objectid, E1.bytes_changed,                           BTRFS_QGROUP_RSV_DATA); 
+ btrfs_qgroup_free_refroot(BTRFS_I(E0)->root->fs_info,                           BTRFS_I(E0)->root->root_key.objectid,                           E1.bytes_changed, BTRFS_QGROUP_RSV_DATA); 
// Infered from: (./EXP0-7/1533533124_2018-08-06_4fd786e6c3d6_qgroup___btrfs_qgroup_release_data.{c.sanitized.c,res.c.sanitized.res.c}: __btrfs_qgroup_release_data)
// Recall: 0.50, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 2 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 2/2(100%)


// ---------------------------------------------
