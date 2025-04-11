static void batadv_bla_periodic_work(struct work_struct *work)
{
  struct delayed_work *delayed_work;
  struct batadv_priv *bat_priv;
  struct batadv_priv_bla *priv_bla;
  struct hlist_head *head;
  struct batadv_bla_backbone_gw *backbone_gw;
  struct batadv_hashtable *hash;
  struct batadv_hard_iface *primary_if;
  bool send_loopdetect = false;
  int i;
  delayed_work = to_delayed_work(work);
  priv_bla = container_of(delayed_work, struct batadv_priv_bla, work);
  bat_priv = container_of(priv_bla, struct batadv_priv, bla);
  primary_if = batadv_primary_if_get_selected(bat_priv);
  if (!primary_if)
    goto out;
  batadv_bla_purge_claims(bat_priv, primary_if, 0);
  batadv_bla_purge_backbone_gw(bat_priv, 0);
  if (!atomic_read(&bat_priv->bridge_loop_avoidance))
    goto out;
  if (atomic_dec_and_test(&bat_priv->bla.loopdetect_next))
    {

      random_ether_addr(bat_priv->bla.loopdetect_addr);
      bat_priv->bla.loopdetect_addr[0] = 0xba;
      bat_priv->bla.loopdetect_addr[1] = 0xbe;
      bat_priv->bla.loopdetect_lasttime = jiffies;
      atomic_set(&bat_priv->bla.loopdetect_next, BATADV_BLA_LOOPDETECT_PERIODS);

      send_loopdetect = true;
    }
  hash = bat_priv->bla.backbone_hash;
  if (!hash)
    goto out;
  for (i = 0; i < hash->size; i++)
    {
      head = &hash->table[i];
      rcu_read_lock();
      hlist_for_each_entry_rcu(backbone_gw, head, hash_entry)
      {
        if (!batadv_compare_eth(backbone_gw->orig, primary_if->net_dev->dev_addr))
          continue;
        backbone_gw->lasttime = jiffies;
        batadv_bla_send_announce(bat_priv, backbone_gw);
        if (send_loopdetect)
          batadv_bla_send_loopdetect(bat_priv, backbone_gw);

        if (atomic_read(&backbone_gw->request_sent) == 0)
          continue;
        if (!atomic_dec_and_test(&backbone_gw->wait_periods))
          continue;
        atomic_dec(&backbone_gw->bat_priv->bla.num_requests);
        atomic_set(&backbone_gw->request_sent, 0);
      }
      rcu_read_unlock();
    }
out:
  if (primary_if)
    batadv_hardif_put(primary_if);
  queue_delayed_work(batadv_event_workqueue, &bat_priv->bla.work, msecs_to_jiffies(BATADV_BLA_PERIOD_LENGTH));
}