static void __init xdbc_free_ring(struct xdbc_ring *ring)
{
	struct xdbc_segment *seg = ring->segment;
	if (!seg)
		return;
	free_bootmem(seg->dma, PAGE_SIZE);
	ring->segment = NULL;
}