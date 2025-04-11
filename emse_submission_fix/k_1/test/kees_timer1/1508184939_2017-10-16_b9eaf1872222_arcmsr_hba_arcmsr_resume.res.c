static int arcmsr_resume(struct pci_dev *pdev)
{
	int error;
	struct Scsi_Host *host = pci_get_drvdata(pdev);
	struct AdapterControlBlock *acb =
		(struct AdapterControlBlock *)host->hostdata;
	pci_set_power_state(pdev, PCI_D0);
	pci_enable_wake(pdev, PCI_D0, 0);
	pci_restore_state(pdev);
	if (pci_enable_device(pdev)) {
		pr_warn("%s: pci_enable_device error\n", __func__);
		return -ENODEV;
	}
	error = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if (error) {
		error = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		if (error) {
			pr_warn("scsi%d: No suitable DMA mask available\n",
			       host->host_no);
			goto controller_unregister;
		}
	}
	pci_set_master(pdev);
	if (arcmsr_request_irq(pdev, acb) == FAILED)
		goto controller_stop;
	arcmsr_iop_init(acb);
	INIT_WORK(&acb->arcmsr_do_message_isr_bh, arcmsr_message_isr_bh_fn);
	atomic_set(&acb->rq_map_token, 16);
	atomic_set(&acb->ante_token_value, 16);
	acb->fw_flag = FW_NORMAL;
	setup_timer(&acb->eternal_timer, &arcmsr_request_device_map,
		    (unsigned long)acb);
	acb->eternal_timer.expires = jiffies + msecs_to_jiffies(6 * HZ);
	add_timer(&acb->eternal_timer);
	return 0;
controller_stop:
	arcmsr_stop_adapter_bgrb(acb);
	arcmsr_flush_adapter_cache(acb);
controller_unregister:
	scsi_remove_host(host);
	arcmsr_free_ccb_pool(acb);
	arcmsr_unmap_pciregion(acb);
	pci_release_regions(pdev);
	scsi_host_put(host);
	pci_disable_device(pdev);
	return -ENODEV;
}