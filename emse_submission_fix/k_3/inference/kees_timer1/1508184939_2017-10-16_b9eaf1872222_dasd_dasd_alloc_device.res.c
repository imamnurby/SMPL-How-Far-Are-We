struct dasd_device *dasd_alloc_device(void)
{
	struct dasd_device *device;
	device = kzalloc(sizeof(struct dasd_device), GFP_ATOMIC);
	if (!device)
		return ERR_PTR(-ENOMEM);
	/* Get two pages for normal block device operations. */
	device->ccw_mem = (void *) __get_free_pages(GFP_ATOMIC | GFP_DMA, 1);
	if (!device->ccw_mem) {
		kfree(device);
		return ERR_PTR(-ENOMEM);
	}
	/* Get one page for error recovery. */
	device->erp_mem = (void *) get_zeroed_page(GFP_ATOMIC | GFP_DMA);
	if (!device->erp_mem) {
		free_pages((unsigned long) device->ccw_mem, 1);
		kfree(device);
		return ERR_PTR(-ENOMEM);
	}
	dasd_init_chunklist(&device->ccw_chunks, device->ccw_mem, PAGE_SIZE*2);
	dasd_init_chunklist(&device->erp_chunks, device->erp_mem, PAGE_SIZE);
	spin_lock_init(&device->mem_lock);
	atomic_set(&device->tasklet_scheduled, 0);
	tasklet_init(&device->tasklet,
		     (void (*)(unsigned long)) dasd_device_tasklet,
		     (unsigned long) device);
	INIT_LIST_HEAD(&device->ccw_queue);
	setup_timer(&device->timer, dasd_device_timeout,
		    (unsigned long)device);
	INIT_WORK(&device->kick_work, do_kick_device);
	INIT_WORK(&device->restore_device, do_restore_device);
	INIT_WORK(&device->reload_device, do_reload_device);
	INIT_WORK(&device->requeue_requests, do_requeue_requests);
	device->state = DASD_STATE_NEW;
	device->target = DASD_STATE_NEW;
	mutex_init(&device->state_mutex);
	spin_lock_init(&device->profile.lock);
	return device;
}