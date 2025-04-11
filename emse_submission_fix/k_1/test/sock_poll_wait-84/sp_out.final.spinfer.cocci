@@
identifier I0, I1, I2;
expression E3, E4;
typedef __poll_t;
typedef poll_table;
@@
  __poll_t tcp_poll(struct file *I0, struct socket *I1, poll_table *I2)  
  {
  ...
- sock_poll_wait(E3, E4); 
+ sock_poll_wait(E3, I1, E4); 
  ...
  }
// Infered from: (./sock_poll_wait-84/1540294839_2018-10-23_89ab066d4229_tcp_tcp_poll.{c,res.c}: tcp_poll), (./sock_poll_wait-84/1540294839_2018-10-23_89ab066d4229_tcp_tcp_poll.{c,res.c}: tcp_poll)
// Recall: 1.00, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 1/1(100%)


// ---------------------------------------------
