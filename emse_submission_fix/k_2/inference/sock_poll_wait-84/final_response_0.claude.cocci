@@
expression file, wait;
identifier sock;
@@
(
struct socket *sock;
|
struct socket *sock = ...;
|
sock = ...;
)
...
- sock_poll_wait(file, wait);
+ sock_poll_wait(file, sock, wait);