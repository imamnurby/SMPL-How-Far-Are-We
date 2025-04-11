static int mt8173_afe_hdmi_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_pcm_runtime *const runtime = substream->runtime;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  struct mt8173_afe_private *afe_priv = afe->platform_priv;
  unsigned int val;
  mt8173_afe_dais_set_clks(afe, afe_priv->clocks[MT8173_CLK_I2S3_M], runtime->rate * 128, afe_priv->clocks[MT8173_CLK_I2S3_B], runtime->rate * runtime->channels * 32);
  val = AFE_TDM_CON1_BCK_INV | AFE_TDM_CON1_LRCK_INV | AFE_TDM_CON1_1_BCK_DELAY | AFE_TDM_CON1_MSB_ALIGNED | AFE_TDM_CON1_WLEN_32BIT | AFE_TDM_CON1_32_BCK_CYCLES | AFE_TDM_CON1_LRCK_WIDTH(32);
  regmap_update_bits(afe->regmap, AFE_TDM_CON1, ~AFE_TDM_CON1_EN, val);

  switch (runtime->channels)
    {
    case 1:
    case 2:
      val = AFE_TDM_CH_START_O30_O31;
      val |= (AFE_TDM_CH_ZERO << 4);
      val |= (AFE_TDM_CH_ZERO << 8);
      val |= (AFE_TDM_CH_ZERO << 12);
      break;
    case 3:
    case 4:
      val = AFE_TDM_CH_START_O30_O31;
      val |= (AFE_TDM_CH_START_O32_O33 << 4);
      val |= (AFE_TDM_CH_ZERO << 8);
      val |= (AFE_TDM_CH_ZERO << 12);
      break;
    case 5:
    case 6:
      val = AFE_TDM_CH_START_O30_O31;
      val |= (AFE_TDM_CH_START_O32_O33 << 4);
      val |= (AFE_TDM_CH_START_O34_O35 << 8);
      val |= (AFE_TDM_CH_ZERO << 12);
      break;
    case 7:
    case 8:
      val = AFE_TDM_CH_START_O30_O31;
      val |= (AFE_TDM_CH_START_O32_O33 << 4);
      val |= (AFE_TDM_CH_START_O34_O35 << 8);
      val |= (AFE_TDM_CH_START_O36_O37 << 12);
      break;
    default:
      val = 0;
    }
  regmap_update_bits(afe->regmap, AFE_TDM_CON2, 0x0000ffff, val);
  regmap_update_bits(afe->regmap, AFE_HDMI_OUT_CON0, 0x000000f0, runtime->channels << 4);
  return 0;
}