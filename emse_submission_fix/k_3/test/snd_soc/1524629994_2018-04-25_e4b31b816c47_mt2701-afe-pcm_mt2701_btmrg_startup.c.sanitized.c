static int mt2701_btmrg_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  struct mt2701_afe_private *afe_priv = afe->platform_priv;
  int ret;
  ret = mt2701_enable_btmrg_clk(afe);
  if (ret)
    return ret;
  afe_priv->mrg_enable[substream->stream] = 1;
  return 0;
}