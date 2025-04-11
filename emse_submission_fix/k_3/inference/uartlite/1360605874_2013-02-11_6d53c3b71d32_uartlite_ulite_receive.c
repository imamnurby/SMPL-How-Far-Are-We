static int ulite_receive(struct uart_port *port, int stat)
{
	struct tty_port *tport = &port->state->port;
	unsigned char ch = 0;
	char flag = TTY_NORMAL;
	if ((stat & (ULITE_STATUS_RXVALID | ULITE_STATUS_OVERRUN
		     | ULITE_STATUS_FRAME)) == 0)
		return 0;
	/* stats */
	if (stat & ULITE_STATUS_RXVALID) {
		port->icount.rx++;
		ch = ioread32be(port->membase + ULITE_RX);
		if (stat & ULITE_STATUS_PARITY)
			port->icount.parity++;
	}
	if (stat & ULITE_STATUS_OVERRUN)
		port->icount.overrun++;
	if (stat & ULITE_STATUS_FRAME)
		port->icount.frame++;
	/* drop byte with parity error if IGNPAR specificed */
	if (stat & port->ignore_status_mask & ULITE_STATUS_PARITY)
		stat &= ~ULITE_STATUS_RXVALID;
	stat &= port->read_status_mask;
	if (stat & ULITE_STATUS_PARITY)
		flag = TTY_PARITY;
	stat &= ~port->ignore_status_mask;
	if (stat & ULITE_STATUS_RXVALID)
		tty_insert_flip_char(tport, ch, flag);
	if (stat & ULITE_STATUS_FRAME)
		tty_insert_flip_char(tport, 0, TTY_FRAME);
	if (stat & ULITE_STATUS_OVERRUN)
		tty_insert_flip_char(tport, 0, TTY_OVERRUN);
	return 1;
}