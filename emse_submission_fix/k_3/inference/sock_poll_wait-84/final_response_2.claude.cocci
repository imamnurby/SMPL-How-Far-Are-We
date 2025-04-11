@@
identifier poll_fn;
struct file *file;
struct socket *sock;
poll_table *wait;
@@
// Find functions matching the poll function signature
function poll_fn(...) {
  ...
- sock_poll_wait(file, wait);
+ sock_poll_wait(file, sock, wait);
  ...
}