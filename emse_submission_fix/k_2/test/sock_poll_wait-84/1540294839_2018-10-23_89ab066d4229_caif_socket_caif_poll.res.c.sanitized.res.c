static __poll_t caif_poll(struct file *file, struct socket *sock, poll_table *wait)
{
  struct sock *sk = sock->sk;
  __poll_t mask;
  struct caifsock *cf_sk = container_of(sk, struct caifsock, sk);
  sock_poll_wait(file, sock, wait);
  mask = 0;

  if (sk->sk_err)
    mask |= EPOLLERR;
  if (sk->sk_shutdown == SHUTDOWN_MASK)
    mask |= EPOLLHUP;
  if (sk->sk_shutdown & RCV_SHUTDOWN)
    mask |= EPOLLRDHUP;

  if (!skb_queue_empty(&sk->sk_receive_queue) || (sk->sk_shutdown & RCV_SHUTDOWN))
    mask |= EPOLLIN | EPOLLRDNORM;

  if (sock_writeable(sk) && tx_flow_is_on(cf_sk))
    mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
  return mask;
}