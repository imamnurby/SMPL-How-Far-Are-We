static int mt2701_dlm_fe_trigger(struct snd_pcm_substream *substream, int cmd, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  struct mtk_base_afe_memif *memif_tmp = &afe->memif[MT2701_MEMIF_DL1];
  switch (cmd)
    {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
      regmap_update_bits(afe->regmap, memif_tmp->data->enable_reg, 1 << memif_tmp->data->enable_shift, 1 << memif_tmp->data->enable_shift);
      mtk_afe_fe_trigger(substream, cmd, dai);
      return 0;
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
      mtk_afe_fe_trigger(substream, cmd, dai);
      regmap_update_bits(afe->regmap, memif_tmp->data->enable_reg, 1 << memif_tmp->data->enable_shift, 0);
      return 0;
    default:
      return -EINVAL;
    }
}