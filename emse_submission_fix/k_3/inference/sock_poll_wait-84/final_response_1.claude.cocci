@@
identifier poll_fn;
expression file, sock, wait;
@@
poll_fn(struct file *file, struct socket *sock, poll_table *wait)
{
  <...
- sock_poll_wait(file, wait);
+ sock_poll_wait(file, sock, wait);
  ...>
}