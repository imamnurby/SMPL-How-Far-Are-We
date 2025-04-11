@@
expression priv, exts, parse_attr, flow;
identifier a;
@@
 static int parse_tc_fdb_actions(struct mlx5e_priv *priv, struct tcf_exts *exts, struct mlx5e_tc_flow_parse_attr *parse_attr, struct mlx5e_tc_flow *flow)
 {
   struct mlx5_esw_flow_attr *attr = flow->esw_attr;
   struct mlx5e_rep_priv *rpriv = priv->ppriv;
   struct ip_tunnel_info *info = NULL;
   const struct tc_action *a;
-  LIST_HEAD(actions);
+  int i;
   bool encap = false;
   u32 action = 0;
   int err;
   if (!tcf_exts_has_actions(exts))
     return -EINVAL;
   attr->in_rep = rpriv->rep;
   attr->in_mdev = priv->mdev;
-  tcf_exts_to_list(exts, &actions);
-  list_for_each_entry(a, &actions, list)
+  tcf_exts_for_each_action(i, a, exts)
   {
     ...
   }
   ...
 }