@@
type T;
identifier F;
expression dev, mem, num_pages, align, acc_size, placement, robj, destroy;
@@
- F(dev, mem, num_pages, placement, align, false, NULL, acc_size, robj, destroy);
+ F(dev, mem, num_pages, placement, align, false, acc_size, robj, destroy);