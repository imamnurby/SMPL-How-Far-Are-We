static __poll_t rxrpc_poll(struct file *file, struct socket *sock, poll_table *wait)
{
  struct sock *sk = sock->sk;
  struct rxrpc_sock *rx = rxrpc_sk(sk);
  __poll_t mask;
  sock_poll_wait(file, wait);
  mask = 0;

  if (!list_empty(&rx->recvmsg_q))
    mask |= EPOLLIN | EPOLLRDNORM;

  if (rxrpc_writable(sk))
    mask |= EPOLLOUT | EPOLLWRNORM;
  return mask;
}