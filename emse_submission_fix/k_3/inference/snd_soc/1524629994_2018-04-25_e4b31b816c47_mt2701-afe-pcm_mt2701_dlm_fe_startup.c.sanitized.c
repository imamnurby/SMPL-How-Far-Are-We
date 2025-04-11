static int mt2701_dlm_fe_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  struct mtk_base_afe_memif *memif_tmp;
  const struct mtk_base_memif_data *memif_data;
  int i;
  for (i = MT2701_MEMIF_DL1; i < MT2701_MEMIF_DL_SINGLE_NUM; ++i)
    {
      memif_tmp = &afe->memif[i];
      if (memif_tmp->substream)
        return -EBUSY;
    }

  for (i = MT2701_MEMIF_DL1; i < MT2701_MEMIF_DL_SINGLE_NUM; ++i)
    {
      memif_data = afe->memif[i].data;
      regmap_update_bits(afe->regmap, memif_data->agent_disable_reg, 1 << memif_data->agent_disable_shift, 0 << memif_data->agent_disable_shift);
    }
  return mtk_afe_fe_startup(substream, dai);
}