@@
identifier f, sock_fn;
expression file, wait;
@@
f(...) {
  <...
- sock_poll_wait(file, wait)
+ sock_poll_wait(file, sock_fn, wait)
  ...>
}


