static int mt2701_afe_i2s_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  int clk_domain;
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  struct mt2701_afe_private *afe_priv = afe->platform_priv;
  int i2s_num = mt2701_dai_num_to_i2s(afe, dai->id);
  struct mt2701_i2s_path *i2s_path;
  int mclk_rate;
  if (i2s_num < 0)
    return i2s_num;
  i2s_path = &afe_priv->i2s_path[i2s_num];
  mclk_rate = i2s_path->mclk_rate;
  if (i2s_path->occupied[substream->stream])
    return -EBUSY;
  i2s_path->occupied[substream->stream] = 1;
  if (MT2701_PLL_DOMAIN_0_RATE % mclk_rate == 0)
    {
      clk_domain = 0;
    }
  else if (MT2701_PLL_DOMAIN_1_RATE % mclk_rate == 0)
    {
      clk_domain = 1;
    }
  else
    {
      dev_err(dai->dev, "%s() bad mclk rate %d\n", __func__, mclk_rate);
      return -EINVAL;
    }
  mt2701_mclk_configuration(afe, i2s_num, clk_domain, mclk_rate);
  if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
      mt2701_i2s_path_prepare_enable(substream, dai, i2s_num, 0);
    }
  else
    {

      mt2701_i2s_path_prepare_enable(substream, dai, i2s_num, 1);

      mt2701_i2s_path_prepare_enable(substream, dai, i2s_num, 0);
    }
  return 0;
}