static int __init isdn_init(void)
{
	int i;
	char tmprev[50];
	dev = vzalloc(sizeof(isdn_dev));
	if (!dev) {
		printk(KERN_WARNING "isdn: Could not allocate device-struct.\n");
		return -EIO;
	}
	init_timer(&dev->timer);
	dev->timer.function = isdn_timer_funct;
	spin_lock_init(&dev->lock);
	spin_lock_init(&dev->timerlock);
#ifdef MODULE
	dev->owner = THIS_MODULE;
#endif
	mutex_init(&dev->mtx);
	init_waitqueue_head(&dev->info_waitq);
	for (i = 0; i < ISDN_MAX_CHANNELS; i++) {
		dev->drvmap[i] = -1;
		dev->chanmap[i] = -1;
		dev->m_idx[i] = -1;
		strcpy(dev->num[i], "???");
	}
	if (register_chrdev(ISDN_MAJOR, "isdn", &isdn_fops)) {
		printk(KERN_WARNING "isdn: Could not register control devices\n");
		vfree(dev);
		return -EIO;
	}
	if ((isdn_tty_modem_init()) < 0) {
		printk(KERN_WARNING "isdn: Could not register tty devices\n");
		vfree(dev);
		unregister_chrdev(ISDN_MAJOR, "isdn");
		return -EIO;
	}
#ifdef CONFIG_ISDN_PPP
	if (isdn_ppp_init() < 0) {
		printk(KERN_WARNING "isdn: Could not create PPP-device-structs\n");
		isdn_tty_exit();
		unregister_chrdev(ISDN_MAJOR, "isdn");
		vfree(dev);
		return -EIO;
	}
#endif                          /* CONFIG_ISDN_PPP */
	strcpy(tmprev, isdn_revision);
	printk(KERN_NOTICE "ISDN subsystem Rev: %s/", isdn_getrev(tmprev));
	strcpy(tmprev, isdn_net_revision);
	printk("%s/", isdn_getrev(tmprev));
	strcpy(tmprev, isdn_ppp_revision);
	printk("%s/", isdn_getrev(tmprev));
	strcpy(tmprev, isdn_audio_revision);
	printk("%s/", isdn_getrev(tmprev));
	strcpy(tmprev, isdn_v110_revision);
	printk("%s", isdn_getrev(tmprev));
#ifdef MODULE
	printk(" loaded\n");
#else
	printk("\n");
#endif
	isdn_info_update();
	return 0;
}