static __poll_t llcp_sock_poll(struct file *file, struct socket *sock,
				   poll_table *wait)
{
	struct sock *sk = sock->sk;
	__poll_t mask = 0;
	pr_debug("%p\n", sk);
	sock_poll_wait(file, wait);
	if (sk->sk_state == LLCP_LISTEN)
		return llcp_accept_poll(sk);
	if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue))
		mask |= EPOLLERR |
			(sock_flag(sk, SOCK_SELECT_ERR_QUEUE) ? EPOLLPRI : 0);
	if (!skb_queue_empty(&sk->sk_receive_queue))
		mask |= EPOLLIN | EPOLLRDNORM;
	if (sk->sk_state == LLCP_CLOSED)
		mask |= EPOLLHUP;
	if (sk->sk_shutdown & RCV_SHUTDOWN)
		mask |= EPOLLRDHUP | EPOLLIN | EPOLLRDNORM;
	if (sk->sk_shutdown == SHUTDOWN_MASK)
		mask |= EPOLLHUP;
	if (sock_writeable(sk) && sk->sk_state == LLCP_CONNECTED)
		mask |= EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND;
	else
		sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
	pr_debug("mask 0x%x\n", mask);
	return mask;
}