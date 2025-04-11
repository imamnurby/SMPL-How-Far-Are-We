__poll_t iucv_sock_poll(struct file *file, struct socket *sock, poll_table *wait) {
  struct sock *sk = sock->sk;
  __poll_t mask = 0;
  sock_poll_wait(file, wait);
  if (sk->sk_state == IUCV_LISTEN) return iucv_accept_poll(sk);
  if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue)) mask |= EPOLLERR | (sock_flag(sk, SOCK_SELECT_ERR_QUEUE) ? EPOLLPRI : 0);
  if (sk->sk_shutdown & RCV_SHUTDOWN) mask |= EPOLLRDHUP;
  if (sk->sk_shutdown == SHUTDOWN_MASK) mask |= EPOLLHUP;
  if (!skb_queue_empty(&sk->sk_receive_queue) || (sk->sk_shutdown & RCV_SHUTDOWN)) mask |= EPOLLIN | EPOLLRDNORM;
  if (sk->sk_state == IUCV_CLOSED) mask |= EPOLLHUP;
  if (sk->sk_state == IUCV_DISCONN) mask |= EPOLLIN;
  if (sock_writeable(sk) && iucv_below_msglim(sk))
    mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
  else
    sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
  return mask;
}