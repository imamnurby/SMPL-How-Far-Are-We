static __poll_t smc_poll(struct file *file, struct socket *sock, poll_table *wait)
{
  struct sock *sk = sock->sk;
  __poll_t mask = 0;
  struct smc_sock *smc;
  if (!sk)
    return EPOLLNVAL;
  smc = smc_sk(sock->sk);
  if (smc->use_fallback)
    {

      mask = smc->clcsock->ops->poll(file, smc->clcsock, wait);
      sk->sk_err = smc->clcsock->sk->sk_err;
      if (sk->sk_err)
        mask |= EPOLLERR;
    }
  else
    {
      if (sk->sk_state != SMC_CLOSED)
        sock_poll_wait(file, sock, wait);
      if (sk->sk_err)
        mask |= EPOLLERR;
      if ((sk->sk_shutdown == SHUTDOWN_MASK) || (sk->sk_state == SMC_CLOSED))
        mask |= EPOLLHUP;
      if (sk->sk_state == SMC_LISTEN)
        {

          mask = smc_accept_poll(sk);
        }
      else
        {
          if (atomic_read(&smc->conn.sndbuf_space) || sk->sk_shutdown & SEND_SHUTDOWN)
            {
              mask |= EPOLLOUT | EPOLLWRNORM;
            }
          else
            {
              sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
              set_bit(SOCK_NOSPACE, &sk->sk_socket->flags);
            }
          if (atomic_read(&smc->conn.bytes_to_rcv))
            mask |= EPOLLIN | EPOLLRDNORM;
          if (sk->sk_shutdown & RCV_SHUTDOWN)
            mask |= EPOLLIN | EPOLLRDNORM | EPOLLRDHUP;
          if (sk->sk_state == SMC_APPCLOSEWAIT1)
            mask |= EPOLLIN;
          if (smc->conn.urg_state == SMC_URG_VALID)
            mask |= EPOLLPRI;
        }
    }
  return mask;
}