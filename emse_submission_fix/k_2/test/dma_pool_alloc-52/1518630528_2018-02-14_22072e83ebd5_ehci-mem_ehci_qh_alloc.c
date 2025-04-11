static struct ehci_qh *ehci_qh_alloc (struct ehci_hcd *ehci, gfp_t flags)
{
	struct ehci_qh		*qh;
	dma_addr_t		dma;
	qh = kzalloc(sizeof *qh, GFP_ATOMIC);
	if (!qh)
		goto done;
	qh->hw = (struct ehci_qh_hw *)
		dma_pool_alloc(ehci->qh_pool, flags, &dma);
	if (!qh->hw)
		goto fail;
	memset(qh->hw, 0, sizeof *qh->hw);
	qh->qh_dma = dma;
	// INIT_LIST_HEAD (&qh->qh_list);
	INIT_LIST_HEAD (&qh->qtd_list);
	INIT_LIST_HEAD(&qh->unlink_node);
	/* dummy td enables safe urb queuing */
	qh->dummy = ehci_qtd_alloc (ehci, flags);
	if (qh->dummy == NULL) {
		ehci_dbg (ehci, "no dummy td\n");
		goto fail1;
	}
done:
	return qh;
fail1:
	dma_pool_free(ehci->qh_pool, qh->hw, qh->qh_dma);
fail:
	kfree(qh);
	return NULL;
}