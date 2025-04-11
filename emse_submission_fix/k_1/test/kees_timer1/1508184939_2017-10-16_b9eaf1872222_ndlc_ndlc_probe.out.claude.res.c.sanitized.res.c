int ndlc_probe(void *phy_id, struct nfc_phy_ops *phy_ops, struct device *dev, int phy_headroom, int phy_tailroom, struct llt_ndlc **ndlc_id, struct st_nci_se_status *se_status)
{
  struct llt_ndlc *ndlc;
  ndlc = devm_kzalloc(dev, sizeof(struct llt_ndlc), GFP_KERNEL);
  if (!ndlc)
    return -ENOMEM;
  ndlc->ops = phy_ops;
  ndlc->phy_id = phy_id;
  ndlc->dev = dev;
  ndlc->powered = 0;
  *ndlc_id = ndlc;

  setup_timer(&ndlc->t1_timer, ndlc_t1_timeout, (unsigned long)ndlc);
  setup_timer(&ndlc->t2_timer, ndlc_t2_timeout, (unsigned long)ndlc);
  skb_queue_head_init(&ndlc->rcv_q);
  skb_queue_head_init(&ndlc->send_q);
  skb_queue_head_init(&ndlc->ack_pending_q);
  INIT_WORK(&ndlc->sm_work, llt_ndlc_sm_work);
  return st_nci_probe(ndlc, phy_headroom, phy_tailroom, se_status);
}