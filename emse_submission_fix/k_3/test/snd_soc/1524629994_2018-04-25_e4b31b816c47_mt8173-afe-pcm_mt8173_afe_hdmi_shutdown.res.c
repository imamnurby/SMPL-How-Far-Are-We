static void mt8173_afe_hdmi_shutdown(struct snd_pcm_substream *substream,
				     struct snd_soc_dai *dai)
{
	struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mt8173_afe_private *afe_priv = afe->platform_priv;
	if (dai->active)
		return;
	mt8173_afe_dais_disable_clks(afe, afe_priv->clocks[MT8173_CLK_I2S3_M],
				     afe_priv->clocks[MT8173_CLK_I2S3_B]);
}