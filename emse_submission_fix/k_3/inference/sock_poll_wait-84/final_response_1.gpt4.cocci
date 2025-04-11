@@
identifier func_name; // Matches the function name
parameter p1, p2, p3; // Matches the function parameters
@@
<...>
 // Matches contexts where sock_poll_wait is used
- sock_poll_wait(p1, p3);
+ sock_poll_wait(p1, p2, p3);
<...>