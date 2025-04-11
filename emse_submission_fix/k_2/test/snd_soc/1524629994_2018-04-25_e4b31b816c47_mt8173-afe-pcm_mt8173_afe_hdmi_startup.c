static int mt8173_afe_hdmi_startup(struct snd_pcm_substream *substream,
				   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
	struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
	struct mt8173_afe_private *afe_priv = afe->platform_priv;
	if (dai->active)
		return 0;
	mt8173_afe_dais_enable_clks(afe, afe_priv->clocks[MT8173_CLK_I2S3_M],
				    afe_priv->clocks[MT8173_CLK_I2S3_B]);
	return 0;
}