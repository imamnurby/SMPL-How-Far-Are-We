static __poll_t caif_poll(struct file *file, struct socket *sock, poll_table *wait) {
  struct sock *sk = sock->sk;
  __poll_t mask;
  struct caifsock *cf_sk = container_of(sk, struct caifsock, sk);
  sock_poll_wait(file, wait);
  mask = 0;
  /* exceptional events? */
  if (sk->sk_err) mask |= EPOLLERR;
  if (sk->sk_shutdown == SHUTDOWN_MASK) mask |= EPOLLHUP;
  if (sk->sk_shutdown & RCV_SHUTDOWN) mask |= EPOLLRDHUP;
  /* readable? */
  if (!skb_queue_empty(&sk->sk_receive_queue) || (sk->sk_shutdown & RCV_SHUTDOWN)) mask |= EPOLLIN | EPOLLRDNORM;
  /*
   * we set writable also when the other side has shut down the
   * connection. This prevents stuck sockets.
   */
  if (sock_writeable(sk) && tx_flow_is_on(cf_sk)) mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
  return mask;
}