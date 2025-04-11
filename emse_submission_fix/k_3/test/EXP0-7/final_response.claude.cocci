@@
expression E;
@@
// Match and transform root->objectid to root->root_key.objectid
(
- E->objectid
+ E->root_key.objectid
)


