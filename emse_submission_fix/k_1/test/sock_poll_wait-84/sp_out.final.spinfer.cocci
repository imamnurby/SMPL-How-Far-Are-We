@@
identifier I0, I1, I2, I3;
expression E4, E5;
typedef __poll_t;
typedef poll_table;
@@
  __poll_t I0(struct file *I1, struct socket *I2, poll_table *I3)  
  {
  ...
- sock_poll_wait(E4, E5); 
+ sock_poll_wait(E4, I2, E5); 
  ...
  }
// Infered from: (./sock_poll_wait-84/1540294839_2018-10-23_89ab066d4229_tcp_tcp_poll.{c,res.c}: tcp_poll), (./sock_poll_wait-84/1540294839_2018-10-23_89ab066d4229_tcp_tcp_poll.{c,res.c}: tcp_poll), (./sock_poll_wait-84/1540294839_2018-10-23_89ab066d4229_af_rxrpc_rxrpc_poll.{c,res.c}: rxrpc_poll), (./sock_poll_wait-84/1540294839_2018-10-23_89ab066d4229_af_rxrpc_rxrpc_poll.{c,res.c}: rxrpc_poll)
// Recall: 1.00, Precision: 1.00, Matching recall: 1.00

// ---------------------------------------------
// Final metrics (for the combined 1 rules):
// -- Edit Location --
// Recall: 1.00, Precision: 1.00
// -- Node Change --
// Recall: 1.00, Precision: 1.00
// -- General --
// Functions fully changed: 2/2(100%)


// ---------------------------------------------
