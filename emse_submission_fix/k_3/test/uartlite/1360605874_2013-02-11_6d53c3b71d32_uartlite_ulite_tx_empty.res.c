static unsigned int ulite_tx_empty(struct uart_port *port)
{
	unsigned long flags;
	unsigned int ret;
	spin_lock_irqsave(&port->lock, flags);
	ret = uart_in32(ULITE_STATUS, port);
	spin_unlock_irqrestore(&port->lock, flags);
	return ret & ULITE_STATUS_TXEMPTY ? TIOCSER_TEMT : 0;
}