static __poll_t rxrpc_poll(struct file *file, struct socket *sock,
			       poll_table *wait)
{
	struct sock *sk = sock->sk;
	struct rxrpc_sock *rx = rxrpc_sk(sk);
	__poll_t mask;
	sock_poll_wait(file, wait);
	mask = 0;
	/* the socket is readable if there are any messages waiting on the Rx
	 * queue */
	if (!list_empty(&rx->recvmsg_q))
		mask |= EPOLLIN | EPOLLRDNORM;
	/* the socket is writable if there is space to add new data to the
	 * socket; there is no guarantee that any particular call in progress
	 * on the socket may have space in the Tx ACK window */
	if (rxrpc_writable(sk))
		mask |= EPOLLOUT | EPOLLWRNORM;
	return mask;
}