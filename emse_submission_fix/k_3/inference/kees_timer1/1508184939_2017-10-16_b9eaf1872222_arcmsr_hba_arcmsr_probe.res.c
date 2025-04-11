static int arcmsr_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct Scsi_Host *host;
	struct AdapterControlBlock *acb;
	uint8_t bus,dev_fun;
	int error;
	error = pci_enable_device(pdev);
	if(error){
		return -ENODEV;
	}
	host = scsi_host_alloc(&arcmsr_scsi_host_template, sizeof(struct AdapterControlBlock));
	if(!host){
    		goto pci_disable_dev;
	}
	error = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if(error){
		error = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		if(error){
			printk(KERN_WARNING
			       "scsi%d: No suitable DMA mask available\n",
			       host->host_no);
			goto scsi_host_release;
		}
	}
	init_waitqueue_head(&wait_q);
	bus = pdev->bus->number;
	dev_fun = pdev->devfn;
	acb = (struct AdapterControlBlock *) host->hostdata;
	memset(acb,0,sizeof(struct AdapterControlBlock));
	acb->pdev = pdev;
	acb->host = host;
	host->max_lun = ARCMSR_MAX_TARGETLUN;
	host->max_id = ARCMSR_MAX_TARGETID;		/*16:8*/
	host->max_cmd_len = 16;	 			/*this is issue of 64bit LBA ,over 2T byte*/
	host->can_queue = ARCMSR_MAX_OUTSTANDING_CMD;
	host->cmd_per_lun = ARCMSR_MAX_CMD_PERLUN;	    
	host->this_id = ARCMSR_SCSI_INITIATOR_ID;
	host->unique_id = (bus << 8) | dev_fun;
	pci_set_drvdata(pdev, host);
	pci_set_master(pdev);
	error = pci_request_regions(pdev, "arcmsr");
	if(error){
		goto scsi_host_release;
	}
	spin_lock_init(&acb->eh_lock);
	spin_lock_init(&acb->ccblist_lock);
	spin_lock_init(&acb->postq_lock);
	spin_lock_init(&acb->doneq_lock);
	spin_lock_init(&acb->rqbuffer_lock);
	spin_lock_init(&acb->wqbuffer_lock);
	acb->acb_flags |= (ACB_F_MESSAGE_WQBUFFER_CLEARED |
			ACB_F_MESSAGE_RQBUFFER_CLEARED |
			ACB_F_MESSAGE_WQBUFFER_READED);
	acb->acb_flags &= ~ACB_F_SCSISTOPADAPTER;
	INIT_LIST_HEAD(&acb->ccb_free_list);
	acb->adapter_type = id->driver_data;
	error = arcmsr_remap_pciregion(acb);
	if(!error){
		goto pci_release_regs;
	}
	error = arcmsr_alloc_io_queue(acb);
	if (!error)
		goto unmap_pci_region;
	error = arcmsr_get_firmware_spec(acb);
	if(!error){
		goto free_hbb_mu;
	}
	error = arcmsr_alloc_ccb_pool(acb);
	if(error){
		goto free_hbb_mu;
	}
	error = scsi_add_host(host, &pdev->dev);
	if(error){
		goto free_ccb_pool;
	}
	if (arcmsr_request_irq(pdev, acb) == FAILED)
		goto scsi_host_remove;
	arcmsr_iop_init(acb);
	INIT_WORK(&acb->arcmsr_do_message_isr_bh, arcmsr_message_isr_bh_fn);
	atomic_set(&acb->rq_map_token, 16);
	atomic_set(&acb->ante_token_value, 16);
	acb->fw_flag = FW_NORMAL;
	setup_timer(&acb->eternal_timer, &arcmsr_request_device_map,
		    (unsigned long)acb);
	acb->eternal_timer.expires = jiffies + msecs_to_jiffies(6 * HZ);
	add_timer(&acb->eternal_timer);
	if(arcmsr_alloc_sysfs_attr(acb))
		goto out_free_sysfs;
	scsi_scan_host(host);
	return 0;
out_free_sysfs:
	del_timer_sync(&acb->eternal_timer);
	flush_work(&acb->arcmsr_do_message_isr_bh);
	arcmsr_stop_adapter_bgrb(acb);
	arcmsr_flush_adapter_cache(acb);
	arcmsr_free_irq(pdev, acb);
scsi_host_remove:
	scsi_remove_host(host);
free_ccb_pool:
	arcmsr_free_ccb_pool(acb);
free_hbb_mu:
	arcmsr_free_mu(acb);
unmap_pci_region:
	arcmsr_unmap_pciregion(acb);
pci_release_regs:
	pci_release_regions(pdev);
scsi_host_release:
	scsi_host_put(host);
pci_disable_dev:
	pci_disable_device(pdev);
	return -ENODEV;
}