void st21nfca_se_init(struct nfc_hci_dev *hdev)
{
	struct st21nfca_hci_info *info = nfc_hci_get_clientdata(hdev);
	init_completion(&info->se_info.req_completion);
	/* initialize timers */
	init_timer(&info->se_info.bwi_timer);
	info->se_info.bwi_timer.data = (unsigned long)info;
	info->se_info.bwi_timer.function = st21nfca_se_wt_timeout;
	info->se_info.bwi_active = false;
	init_timer(&info->se_info.se_active_timer);
	info->se_info.se_active_timer.data = (unsigned long)info;
	info->se_info.se_active_timer.function = st21nfca_se_activation_timeout;
	info->se_info.se_active = false;
	info->se_info.count_pipes = 0;
	info->se_info.expected_pipes = 0;
	info->se_info.xch_error = false;
	info->se_info.wt_timeout =
			ST21NFCA_BWI_TO_TIMEOUT(ST21NFCA_ATR_DEFAULT_BWI);
}