@@
field old_field = objectid;
field new_field = root_key.objectid;
identifier root, field_base;
@@
(
- root->old_field
+ root->new_field
|
- root->field_base.old_field
+ root->field_base.new_field
)