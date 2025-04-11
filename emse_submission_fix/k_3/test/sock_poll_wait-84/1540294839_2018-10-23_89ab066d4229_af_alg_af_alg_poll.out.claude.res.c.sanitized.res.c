__poll_t af_alg_poll(struct file *file, struct socket *sock, poll_table *wait)
{
  struct sock *sk = sock->sk;
  struct alg_sock *ask = alg_sk(sk);
  struct af_alg_ctx *ctx = ask->private;
  __poll_t mask;
  sock_poll_wait(file, sock, wait);
  mask = 0;
  if (!ctx->more || ctx->used)
    mask |= EPOLLIN | EPOLLRDNORM;
  if (af_alg_writable(sk))
    mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
  return mask;
}