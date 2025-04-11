__poll_t vcc_poll(struct file *file, struct socket *sock, poll_table *wait) {
  struct sock *sk = sock->sk;
  struct atm_vcc *vcc;
  __poll_t mask;
  sock_poll_wait(file, sock, wait);
  mask = 0;
  vcc = ATM_SD(sock);
  /* exceptional events */
  if (sk->sk_err) mask = EPOLLERR;
  if (test_bit(ATM_VF_RELEASED, &vcc->flags) || test_bit(ATM_VF_CLOSE, &vcc->flags)) mask |= EPOLLHUP;
  /* readable? */
  if (!skb_queue_empty(&sk->sk_receive_queue)) mask |= EPOLLIN | EPOLLRDNORM;
  /* writable? */
  if (sock->state == SS_CONNECTING && test_bit(ATM_VF_WAITING, &vcc->flags)) return mask;
  if (vcc->qos.txtp.traffic_class != ATM_NONE && vcc_writable(sk)) mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
  return mask;
}