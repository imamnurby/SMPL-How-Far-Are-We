static void mt2701_afe_i2s_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  struct mt2701_afe_private *afe_priv = afe->platform_priv;
  int i2s_num = mt2701_dai_num_to_i2s(afe, dai->id);
  struct mt2701_i2s_path *i2s_path;
  if (i2s_num < 0)
    return;
  i2s_path = &afe_priv->i2s_path[i2s_num];
  if (i2s_path->occupied[substream->stream])
    i2s_path->occupied[substream->stream] = 0;
  else
    goto I2S_UNSTART;
  mt2701_afe_i2s_path_shutdown(substream, dai, i2s_num, 0);

  if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
    mt2701_afe_i2s_path_shutdown(substream, dai, i2s_num, 1);
I2S_UNSTART:

  mt2701_afe_disable_mclk(afe, i2s_num);
}