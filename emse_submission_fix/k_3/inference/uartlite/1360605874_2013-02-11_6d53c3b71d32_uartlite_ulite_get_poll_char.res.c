static int ulite_get_poll_char(struct uart_port *port)
{
	if (!(uart_in32(ULITE_STATUS, port)
						& ULITE_STATUS_RXVALID))
		return NO_POLL_CHAR;
	return uart_in32(ULITE_RX, port);
}