void mtk_afe_fe_shutdown(struct snd_pcm_substream *substream,
			 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mtk_base_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	int irq_id;
	irq_id = memif->irq_usage;
	mtk_regmap_update_bits(afe->regmap, memif->data->agent_disable_reg,
			       1 << memif->data->agent_disable_shift,
			       1 << memif->data->agent_disable_shift);
	if (!memif->const_irq) {
		mtk_dynamic_irq_release(afe, irq_id);
		memif->irq_usage = -1;
		memif->substream = NULL;
	}
}