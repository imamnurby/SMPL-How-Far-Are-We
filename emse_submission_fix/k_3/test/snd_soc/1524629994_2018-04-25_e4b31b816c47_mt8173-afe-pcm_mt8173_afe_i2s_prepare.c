static int mt8173_afe_i2s_prepare(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
	struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
	struct mt8173_afe_private *afe_priv = afe->platform_priv;
	int ret;
	mt8173_afe_dais_set_clks(afe, afe_priv->clocks[MT8173_CLK_I2S1_M],
				 runtime->rate * 256, NULL, 0);
	mt8173_afe_dais_set_clks(afe, afe_priv->clocks[MT8173_CLK_I2S2_M],
				 runtime->rate * 256, NULL, 0);
	/* config I2S */
	ret = mt8173_afe_set_i2s(afe, substream->runtime->rate);
	if (ret)
		return ret;
	mt8173_afe_set_i2s_enable(afe, true);
	return 0;
}