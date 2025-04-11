static __poll_t unix_poll(struct file *file, struct socket *sock, poll_table *wait)
{
  struct sock *sk = sock->sk;
  __poll_t mask;
  sock_poll_wait(file, sock, wait);
  mask = 0;

  if (sk->sk_err)
    mask |= EPOLLERR;
  if (sk->sk_shutdown == SHUTDOWN_MASK)
    mask |= EPOLLHUP;
  if (sk->sk_shutdown & RCV_SHUTDOWN)
    mask |= EPOLLRDHUP | EPOLLIN | EPOLLRDNORM;

  if (!skb_queue_empty(&sk->sk_receive_queue))
    mask |= EPOLLIN | EPOLLRDNORM;

  if ((sk->sk_type == SOCK_STREAM || sk->sk_type == SOCK_SEQPACKET) && sk->sk_state == TCP_CLOSE)
    mask |= EPOLLHUP;

  if (unix_writable(sk))
    mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
  return mask;
}