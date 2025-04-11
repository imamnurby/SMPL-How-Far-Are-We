static void mt2701_dlm_fe_shutdown(struct snd_pcm_substream *substream,
				   struct snd_soc_dai *dai)
{
	struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
	const struct mtk_base_memif_data *memif_data;
	int i;
	for (i = MT2701_MEMIF_DL1; i < MT2701_MEMIF_DL_SINGLE_NUM; ++i) {
		memif_data = afe->memif[i].data;
		regmap_update_bits(afe->regmap,
				   memif_data->agent_disable_reg,
				   1 << memif_data->agent_disable_shift,
				   1 << memif_data->agent_disable_shift);
	}
	return mtk_afe_fe_shutdown(substream, dai);
}