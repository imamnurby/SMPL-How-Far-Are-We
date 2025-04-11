int mtk_afe_fe_trigger(struct snd_pcm_substream *substream, int cmd,
		       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mtk_base_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	struct mtk_base_afe_irq *irqs = &afe->irqs[memif->irq_usage];
	const struct mtk_base_irq_data *irq_data = irqs->irq_data;
	unsigned int counter = runtime->period_size;
	int fs;
	dev_dbg(afe->dev, "%s %s cmd=%d\n", __func__, memif->data->name, cmd);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		if (memif->data->enable_shift >= 0)
			mtk_regmap_update_bits(afe->regmap,
					       memif->data->enable_reg,
					       1 << memif->data->enable_shift,
					       1 << memif->data->enable_shift);
		/* set irq counter */
		mtk_regmap_update_bits(afe->regmap, irq_data->irq_cnt_reg,
				       irq_data->irq_cnt_maskbit
				       << irq_data->irq_cnt_shift,
				       counter << irq_data->irq_cnt_shift);
		/* set irq fs */
		fs = afe->irq_fs(substream, runtime->rate);
		if (fs < 0)
			return -EINVAL;
		mtk_regmap_update_bits(afe->regmap, irq_data->irq_fs_reg,
				       irq_data->irq_fs_maskbit
				       << irq_data->irq_fs_shift,
				       fs << irq_data->irq_fs_shift);
		/* enable interrupt */
		mtk_regmap_update_bits(afe->regmap, irq_data->irq_en_reg,
				       1 << irq_data->irq_en_shift,
				       1 << irq_data->irq_en_shift);
		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		mtk_regmap_update_bits(afe->regmap, memif->data->enable_reg,
				       1 << memif->data->enable_shift, 0);
		/* disable interrupt */
		mtk_regmap_update_bits(afe->regmap, irq_data->irq_en_reg,
				       1 << irq_data->irq_en_shift,
				       0 << irq_data->irq_en_shift);
		/* and clear pending IRQ */
		mtk_regmap_write(afe->regmap, irq_data->irq_clr_reg,
				 1 << irq_data->irq_clr_shift);
		return 0;
	default:
		return -EINVAL;
	}
}