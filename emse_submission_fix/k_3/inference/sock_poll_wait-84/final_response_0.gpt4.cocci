@@
identifier file, sock;
expression wait;
@@
- sock_poll_wait(file, wait);
+ sock_poll_wait(file, sock, wait);