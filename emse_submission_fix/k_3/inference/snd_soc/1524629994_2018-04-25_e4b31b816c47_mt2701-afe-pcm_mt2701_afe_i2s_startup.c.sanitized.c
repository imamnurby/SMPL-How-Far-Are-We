static int mt2701_afe_i2s_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  int i2s_num = mt2701_dai_num_to_i2s(afe, dai->id);
  if (i2s_num < 0)
    return i2s_num;
  return mt2701_afe_enable_mclk(afe, i2s_num);
}