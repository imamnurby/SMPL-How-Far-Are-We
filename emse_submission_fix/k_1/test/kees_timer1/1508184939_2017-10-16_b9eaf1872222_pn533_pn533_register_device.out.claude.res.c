struct pn533 *pn533_register_device(u32 device_type, u32 protocols, enum pn533_protocol_type protocol_type, void *phy, struct pn533_phy_ops *phy_ops, struct pn533_frame_ops *fops, struct device *dev, struct device *parent)
{
  struct pn533 *priv;
  int rc = -ENOMEM;
  priv = kzalloc(sizeof(*priv), GFP_KERNEL);
  if (!priv)
    return ERR_PTR(-ENOMEM);
  priv->phy = phy;
  priv->phy_ops = phy_ops;
  priv->dev = dev;
  if (fops != NULL)
    priv->ops = fops;
  else
    priv->ops = &pn533_std_frame_ops;
  priv->protocol_type = protocol_type;
  priv->device_type = device_type;
  mutex_init(&priv->cmd_lock);
  INIT_WORK(&priv->cmd_work, pn533_wq_cmd);
  INIT_WORK(&priv->cmd_complete_work, pn533_wq_cmd_complete);
  INIT_WORK(&priv->mi_rx_work, pn533_wq_mi_recv);
  INIT_WORK(&priv->mi_tx_work, pn533_wq_mi_send);
  INIT_WORK(&priv->tg_work, pn533_wq_tg_get_data);
  INIT_WORK(&priv->mi_tm_rx_work, pn533_wq_tm_mi_recv);
  INIT_WORK(&priv->mi_tm_tx_work, pn533_wq_tm_mi_send);
  INIT_DELAYED_WORK(&priv->poll_work, pn533_wq_poll);
  INIT_WORK(&priv->rf_work, pn533_wq_rf);
  priv->wq = alloc_ordered_workqueue("pn533", 0);
  if (priv->wq == NULL)
    goto error;
  setup_timer(&priv->listen_timer, pn533_listen_mode_timer,
              (unsigned long)priv);
  skb_queue_head_init(&priv->resp_q);
  skb_queue_head_init(&priv->fragment_skb);
  INIT_LIST_HEAD(&priv->cmd_queue);
  priv->nfc_dev = nfc_allocate_device(&pn533_nfc_ops, protocols, priv->ops->tx_header_len + PN533_CMD_DATAEXCH_HEAD_LEN, priv->ops->tx_tail_len);
  if (!priv->nfc_dev)
    {
      rc = -ENOMEM;
      goto destroy_wq;
    }
  nfc_set_parent_dev(priv->nfc_dev, parent);
  nfc_set_drvdata(priv->nfc_dev, priv);
  rc = nfc_register_device(priv->nfc_dev);
  if (rc)
    goto free_nfc_dev;
  return priv;
free_nfc_dev:
  nfc_free_device(priv->nfc_dev);
destroy_wq:
  destroy_workqueue(priv->wq);
error:
  kfree(priv);
  return ERR_PTR(rc);
}