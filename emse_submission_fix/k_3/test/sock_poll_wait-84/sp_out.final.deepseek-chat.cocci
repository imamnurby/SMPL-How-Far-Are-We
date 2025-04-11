@@
expression file, wait;
struct socket *sock;
@@
- sock_poll_wait(file, wait)
+ sock_poll_wait(file, sock, wait)


