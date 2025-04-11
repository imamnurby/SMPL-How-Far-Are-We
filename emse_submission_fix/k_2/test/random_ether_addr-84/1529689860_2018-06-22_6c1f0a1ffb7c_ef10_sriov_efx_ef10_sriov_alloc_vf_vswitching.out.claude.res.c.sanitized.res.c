static int efx_ef10_sriov_alloc_vf_vswitching(struct efx_nic *efx)
{
  struct efx_ef10_nic_data *nic_data = efx->nic_data;
  unsigned int i;
  int rc;
  nic_data->vf = kcalloc(efx->vf_count, sizeof(struct ef10_vf), GFP_KERNEL);
  if (!nic_data->vf)
    return -ENOMEM;
  for (i = 0; i < efx->vf_count; i++)
    {
      eth_random_addr(nic_data->vf[i].mac);
      nic_data->vf[i].efx = NULL;
      nic_data->vf[i].vlan = EFX_EF10_NO_VLAN;
      rc = efx_ef10_sriov_assign_vf_vport(efx, i);
      if (rc)
        goto fail;
    }
  return 0;
fail:
  efx_ef10_sriov_free_vf_vports(efx);
  kfree(nic_data->vf);
  nic_data->vf = NULL;
  return rc;
}