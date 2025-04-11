__poll_t dccp_poll(struct file *file, struct socket *sock, poll_table *wait) {
  __poll_t mask;
  struct sock *sk = sock->sk;
  sock_poll_wait(file, sock, wait);
  if (sk->sk_state == DCCP_LISTEN) return inet_csk_listen_poll(sk);
  /* Socket is not locked. We are protected from async events
     by poll logic and correct handling of state changes
     made by another threads is impossible in any case.
   */
  mask = 0;
  if (sk->sk_err) mask = EPOLLERR;
  if (sk->sk_shutdown == SHUTDOWN_MASK || sk->sk_state == DCCP_CLOSED) mask |= EPOLLHUP;
  if (sk->sk_shutdown & RCV_SHUTDOWN) mask |= EPOLLIN | EPOLLRDNORM | EPOLLRDHUP;
  /* Connected? */
  if ((1 << sk->sk_state) & ~(DCCPF_REQUESTING | DCCPF_RESPOND)) {
    if (atomic_read(&sk->sk_rmem_alloc) > 0) mask |= EPOLLIN | EPOLLRDNORM;
    if (!(sk->sk_shutdown & SEND_SHUTDOWN)) {
      if (sk_stream_is_writeable(sk)) {
        mask |= EPOLLOUT | EPOLLWRNORM;
      } else { /* send SIGIO later */
        sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
        set_bit(SOCK_NOSPACE, &sk->sk_socket->flags);
        /* Race breaker. If space is freed after
         * wspace test but before the flags are set,
         * IO signal will be lost.
         */
        if (sk_stream_is_writeable(sk)) mask |= EPOLLOUT | EPOLLWRNORM;
      }
    }
  }
  return mask;
}