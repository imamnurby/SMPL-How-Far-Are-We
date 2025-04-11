int st_nci_se_init(struct nci_dev *ndev, struct st_nci_se_status *se_status)
{
  struct st_nci_info *info = nci_get_drvdata(ndev);
  init_completion(&info->se_info.req_completion);

  setup_timer(&info->se_info.bwi_timer, st_nci_se_wt_timeout, (unsigned long)info);
  info->se_info.bwi_active = false;
  setup_timer(&info->se_info.se_active_timer, st_nci_se_activation_timeout, (unsigned long)info);
  info->se_info.se_active = false;
  info->se_info.xch_error = false;
  info->se_info.wt_timeout = ST_NCI_BWI_TO_TIMEOUT(ST_NCI_ATR_DEFAULT_BWI);
  info->se_info.se_status = se_status;
  return 0;
}