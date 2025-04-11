static int ulite_get_poll_char(struct uart_port *port)
{
  if (!(ioread32be(port->membase + ULITE_STATUS) & ULITE_STATUS_RXVALID))
    return NO_POLL_CHAR;
  return ioread32be(port->membase + ULITE_RX);
}