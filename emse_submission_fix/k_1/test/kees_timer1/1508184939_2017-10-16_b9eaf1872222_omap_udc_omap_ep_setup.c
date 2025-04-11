static unsigned
omap_ep_setup(char *name, u8 addr, u8 type,
		unsigned buf, unsigned maxp, int dbuf)
{
	struct omap_ep	*ep;
	u16		epn_rxtx = 0;
	/* OUT endpoints first, then IN */
	ep = &udc->ep[addr & 0xf];
	if (addr & USB_DIR_IN)
		ep += 16;
	/* in case of ep init table bugs */
	BUG_ON(ep->name[0]);
	/* chip setup ... bit values are same for IN, OUT */
	if (type == USB_ENDPOINT_XFER_ISOC) {
		switch (maxp) {
		case 8:
			epn_rxtx = 0 << 12;
			break;
		case 16:
			epn_rxtx = 1 << 12;
			break;
		case 32:
			epn_rxtx = 2 << 12;
			break;
		case 64:
			epn_rxtx = 3 << 12;
			break;
		case 128:
			epn_rxtx = 4 << 12;
			break;
		case 256:
			epn_rxtx = 5 << 12;
			break;
		case 512:
			epn_rxtx = 6 << 12;
			break;
		default:
			BUG();
		}
		epn_rxtx |= UDC_EPN_RX_ISO;
		dbuf = 1;
	} else {
		/* double-buffering "not supported" on 15xx,
		 * and ignored for PIO-IN on newer chips
		 * (for more reliable behavior)
		 */
		if (!use_dma || cpu_is_omap15xx())
			dbuf = 0;
		switch (maxp) {
		case 8:
			epn_rxtx = 0 << 12;
			break;
		case 16:
			epn_rxtx = 1 << 12;
			break;
		case 32:
			epn_rxtx = 2 << 12;
			break;
		case 64:
			epn_rxtx = 3 << 12;
			break;
		default:
			BUG();
		}
		if (dbuf && addr)
			epn_rxtx |= UDC_EPN_RX_DB;
		init_timer(&ep->timer);
		ep->timer.function = pio_out_timer;
		ep->timer.data = (unsigned long) ep;
	}
	if (addr)
		epn_rxtx |= UDC_EPN_RX_VALID;
	BUG_ON(buf & 0x07);
	epn_rxtx |= buf >> 3;
	DBG("%s addr %02x rxtx %04x maxp %d%s buf %d\n",
		name, addr, epn_rxtx, maxp, dbuf ? "x2" : "", buf);
	if (addr & USB_DIR_IN)
		omap_writew(epn_rxtx, UDC_EP_TX(addr & 0xf));
	else
		omap_writew(epn_rxtx, UDC_EP_RX(addr));
	/* next endpoint's buffer starts after this one's */
	buf += maxp;
	if (dbuf)
		buf += maxp;
	BUG_ON(buf > 2048);
	/* set up driver data structures */
	BUG_ON(strlen(name) >= sizeof ep->name);
	strlcpy(ep->name, name, sizeof ep->name);
	INIT_LIST_HEAD(&ep->queue);
	INIT_LIST_HEAD(&ep->iso);
	ep->bEndpointAddress = addr;
	ep->bmAttributes = type;
	ep->double_buf = dbuf;
	ep->udc = udc;
	switch (type) {
	case USB_ENDPOINT_XFER_CONTROL:
		ep->ep.caps.type_control = true;
		ep->ep.caps.dir_in = true;
		ep->ep.caps.dir_out = true;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		ep->ep.caps.type_iso = true;
		break;
	case USB_ENDPOINT_XFER_BULK:
		ep->ep.caps.type_bulk = true;
		break;
	case USB_ENDPOINT_XFER_INT:
		ep->ep.caps.type_int = true;
		break;
	};
	if (addr & USB_DIR_IN)
		ep->ep.caps.dir_in = true;
	else
		ep->ep.caps.dir_out = true;
	ep->ep.name = ep->name;
	ep->ep.ops = &omap_ep_ops;
	ep->maxpacket = maxp;
	usb_ep_set_maxpacket_limit(&ep->ep, ep->maxpacket);
	list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);
	return buf;
}