static void ulite_console_wait_tx(struct uart_port *port)
{
  int i;
  u8 val;

  for (i = 0; i < 100000; i++)
    {
      val = uart_in32(ULITE_STATUS, port);
      if ((val & ULITE_STATUS_TXFULL) == 0)
        break;
      cpu_relax();
    }
}