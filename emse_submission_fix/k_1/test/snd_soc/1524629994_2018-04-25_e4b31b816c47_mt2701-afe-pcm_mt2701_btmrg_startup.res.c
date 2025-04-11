static int mt2701_btmrg_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mt2701_afe_private *afe_priv = afe->platform_priv;
	int ret;
	ret = mt2701_enable_btmrg_clk(afe);
	if (ret)
		return ret;
	afe_priv->mrg_enable[substream->stream] = 1;
	return 0;
}