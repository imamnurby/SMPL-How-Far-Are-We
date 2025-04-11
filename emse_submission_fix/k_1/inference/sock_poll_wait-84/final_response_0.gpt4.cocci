@@
identifier f;
expression sock, wait;
@@
- sock_poll_wait(f, wait)
+ sock_poll_wait(f, sock, wait)