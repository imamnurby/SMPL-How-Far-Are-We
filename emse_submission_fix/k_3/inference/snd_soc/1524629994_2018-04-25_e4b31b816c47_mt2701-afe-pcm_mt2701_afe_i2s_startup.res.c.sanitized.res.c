static int mt2701_afe_i2s_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
  int i2s_num = mt2701_dai_num_to_i2s(afe, dai->id);
  if (i2s_num < 0)
    return i2s_num;
  return mt2701_afe_enable_mclk(afe, i2s_num);
}