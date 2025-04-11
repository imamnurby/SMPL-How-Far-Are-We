static void mt8173_afe_i2s_shutdown(struct snd_pcm_substream *substream,
				    struct snd_soc_dai *dai)
{
	struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
	if (dai->active)
		return;
	mt8173_afe_set_i2s_enable(afe, false);
	regmap_update_bits(afe->regmap, AUDIO_TOP_CON0,
			   AUD_TCON0_PDN_22M | AUD_TCON0_PDN_24M,
			   AUD_TCON0_PDN_22M | AUD_TCON0_PDN_24M);
}