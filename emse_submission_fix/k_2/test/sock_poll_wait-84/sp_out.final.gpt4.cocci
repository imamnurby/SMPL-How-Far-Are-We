@@
identifier func, sock;
expression file, wait;
@@
func(...) {  
  ...
- sock_poll_wait(file, wait);
+ sock_poll_wait(file, sock, wait);
  ...
}


