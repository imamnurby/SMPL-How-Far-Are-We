static irqreturn_t ulite_isr(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	int busy, n = 0;
	do {
		int stat = uart_in32(ULITE_STATUS, port);
		busy  = ulite_receive(port, stat);
		busy |= ulite_transmit(port, stat);
		n++;
	} while (busy);
	/* work done? */
	if (n > 1) {
		tty_flip_buffer_push(&port->state->port);
		return IRQ_HANDLED;
	} else {
		return IRQ_NONE;
	}
}