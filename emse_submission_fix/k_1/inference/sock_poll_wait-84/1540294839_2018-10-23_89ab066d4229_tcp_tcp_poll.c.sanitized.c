__poll_t tcp_poll(struct file *file, struct socket *sock, poll_table *wait)
{
  __poll_t mask;
  struct sock *sk = sock->sk;
  const struct tcp_sock *tp = tcp_sk(sk);
  int state;
  sock_poll_wait(file, wait);
  state = inet_sk_state_load(sk);
  if (state == TCP_LISTEN)
    return inet_csk_listen_poll(sk);

  mask = 0;

  if (sk->sk_shutdown == SHUTDOWN_MASK || state == TCP_CLOSE)
    mask |= EPOLLHUP;
  if (sk->sk_shutdown & RCV_SHUTDOWN)
    mask |= EPOLLIN | EPOLLRDNORM | EPOLLRDHUP;

  if (state != TCP_SYN_SENT && (state != TCP_SYN_RECV || tp->fastopen_rsk))
    {
      int target = sock_rcvlowat(sk, 0, INT_MAX);
      if (tp->urg_seq == tp->copied_seq && !sock_flag(sk, SOCK_URGINLINE) && tp->urg_data)
        target++;
      if (tcp_stream_is_readable(tp, target, sk))
        mask |= EPOLLIN | EPOLLRDNORM;
      if (!(sk->sk_shutdown & SEND_SHUTDOWN))
        {
          if (sk_stream_is_writeable(sk))
            {
              mask |= EPOLLOUT | EPOLLWRNORM;
            }
          else
            {
              sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
              set_bit(SOCK_NOSPACE, &sk->sk_socket->flags);

              smp_mb__after_atomic();
              if (sk_stream_is_writeable(sk))
                mask |= EPOLLOUT | EPOLLWRNORM;
            }
        }
      else
        mask |= EPOLLOUT | EPOLLWRNORM;
      if (tp->urg_data & TCP_URG_VALID)
        mask |= EPOLLPRI;
    }
  else if (state == TCP_SYN_SENT && inet_sk(sk)->defer_connect)
    {

      mask |= EPOLLOUT | EPOLLWRNORM;
    }

  smp_rmb();
  if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue))
    mask |= EPOLLERR;
  return mask;
}