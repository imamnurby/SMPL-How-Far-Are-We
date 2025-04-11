__poll_t datagram_poll(struct file *file, struct socket *sock, poll_table *wait)
{
  struct sock *sk = sock->sk;
  __poll_t mask;
  sock_poll_wait(file, wait);
  mask = 0;

  if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue))
    mask |= EPOLLERR | (sock_flag(sk, SOCK_SELECT_ERR_QUEUE) ? EPOLLPRI : 0);
  if (sk->sk_shutdown & RCV_SHUTDOWN)
    mask |= EPOLLRDHUP | EPOLLIN | EPOLLRDNORM;
  if (sk->sk_shutdown == SHUTDOWN_MASK)
    mask |= EPOLLHUP;

  if (!skb_queue_empty(&sk->sk_receive_queue))
    mask |= EPOLLIN | EPOLLRDNORM;

  if (connection_based(sk))
    {
      if (sk->sk_state == TCP_CLOSE)
        mask |= EPOLLHUP;

      if (sk->sk_state == TCP_SYN_SENT)
        return mask;
    }

  if (sock_writeable(sk))
    mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
  else
    sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
  return mask;
}