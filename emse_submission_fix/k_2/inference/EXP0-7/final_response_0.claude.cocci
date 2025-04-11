@@
expression root;
@@
(
- root->objectid
+ root->root_key.objectid
|
// The initialization in the __setup_root function is a special case
- root->objectid = objectid;
+ root->root_key.objectid = objectid;
)