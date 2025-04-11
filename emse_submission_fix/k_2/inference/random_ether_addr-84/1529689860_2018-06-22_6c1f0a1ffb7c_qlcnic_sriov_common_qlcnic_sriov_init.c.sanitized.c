int qlcnic_sriov_init(struct qlcnic_adapter *adapter, int num_vfs)
{
  struct qlcnic_sriov *sriov;
  struct qlcnic_back_channel *bc;
  struct workqueue_struct *wq;
  struct qlcnic_vport *vp;
  struct qlcnic_vf_info *vf;
  int err, i;
  if (!qlcnic_sriov_enable_check(adapter))
    return -EIO;
  sriov = kzalloc(sizeof(struct qlcnic_sriov), GFP_KERNEL);
  if (!sriov)
    return -ENOMEM;
  adapter->ahw->sriov = sriov;
  sriov->num_vfs = num_vfs;
  bc = &sriov->bc;
  sriov->vf_info = kcalloc(num_vfs, sizeof(struct qlcnic_vf_info), GFP_KERNEL);
  if (!sriov->vf_info)
    {
      err = -ENOMEM;
      goto qlcnic_free_sriov;
    }
  wq = create_singlethread_workqueue("bc-trans");
  if (wq == NULL)
    {
      err = -ENOMEM;
      dev_err(&adapter->pdev->dev, "Cannot create bc-trans workqueue\n");
      goto qlcnic_free_vf_info;
    }
  bc->bc_trans_wq = wq;
  wq = create_singlethread_workqueue("async");
  if (wq == NULL)
    {
      err = -ENOMEM;
      dev_err(&adapter->pdev->dev, "Cannot create async workqueue\n");
      goto qlcnic_destroy_trans_wq;
    }
  bc->bc_async_wq = wq;
  INIT_LIST_HEAD(&bc->async_cmd_list);
  INIT_WORK(&bc->vf_async_work, qlcnic_sriov_handle_async_issue_cmd);
  spin_lock_init(&bc->queue_lock);
  bc->adapter = adapter;
  for (i = 0; i < num_vfs; i++)
    {
      vf = &sriov->vf_info[i];
      vf->adapter = adapter;
      vf->pci_func = qlcnic_sriov_virtid_fn(adapter, i);
      mutex_init(&vf->send_cmd_lock);
      spin_lock_init(&vf->vlan_list_lock);
      INIT_LIST_HEAD(&vf->rcv_act.wait_list);
      INIT_LIST_HEAD(&vf->rcv_pend.wait_list);
      spin_lock_init(&vf->rcv_act.lock);
      spin_lock_init(&vf->rcv_pend.lock);
      init_completion(&vf->ch_free_cmpl);
      INIT_WORK(&vf->trans_work, qlcnic_sriov_process_bc_cmd);
      if (qlcnic_sriov_pf_check(adapter))
        {
          vp = kzalloc(sizeof(struct qlcnic_vport), GFP_KERNEL);
          if (!vp)
            {
              err = -ENOMEM;
              goto qlcnic_destroy_async_wq;
            }
          sriov->vf_info[i].vp = vp;
          vp->vlan_mode = QLC_GUEST_VLAN_MODE;
          vp->max_tx_bw = MAX_BW;
          vp->min_tx_bw = MIN_BW;
          vp->spoofchk = false;
          random_ether_addr(vp->mac);
          dev_info(&adapter->pdev->dev, "MAC Address %pM is configured for VF %d\n", vp->mac, i);
        }
    }
  return 0;
qlcnic_destroy_async_wq:
  destroy_workqueue(bc->bc_async_wq);
qlcnic_destroy_trans_wq:
  destroy_workqueue(bc->bc_trans_wq);
qlcnic_free_vf_info:
  kfree(sriov->vf_info);
qlcnic_free_sriov:
  kfree(adapter->ahw->sriov);
  return err;
}