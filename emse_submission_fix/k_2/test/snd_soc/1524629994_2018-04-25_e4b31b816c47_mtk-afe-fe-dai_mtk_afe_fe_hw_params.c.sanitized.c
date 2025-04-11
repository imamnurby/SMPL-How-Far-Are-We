int mtk_afe_fe_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  struct mtk_base_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
  int msb_at_bit33 = 0;
  int ret, fs = 0;
  ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
  if (ret < 0)
    return ret;
  msb_at_bit33 = upper_32_bits(substream->runtime->dma_addr) ? 1 : 0;
  memif->phys_buf_addr = lower_32_bits(substream->runtime->dma_addr);
  memif->buffer_size = substream->runtime->dma_bytes;

  mtk_regmap_write(afe->regmap, memif->data->reg_ofs_base, memif->phys_buf_addr);

  mtk_regmap_write(afe->regmap, memif->data->reg_ofs_base + AFE_BASE_END_OFFSET, memif->phys_buf_addr + memif->buffer_size - 1);

  mtk_regmap_update_bits(afe->regmap, memif->data->msb_reg, 1 << memif->data->msb_shift, msb_at_bit33 << memif->data->msb_shift);

  if (memif->data->mono_shift >= 0)
    {
      unsigned int mono = (params_channels(params) == 1) ? 1 : 0;
      mtk_regmap_update_bits(afe->regmap, memif->data->mono_reg, 1 << memif->data->mono_shift, mono << memif->data->mono_shift);
    }

  if (memif->data->fs_shift < 0)
    return 0;
  fs = afe->memif_fs(substream, params_rate(params));
  if (fs < 0)
    return -EINVAL;
  mtk_regmap_update_bits(afe->regmap, memif->data->fs_reg, memif->data->fs_maskbit << memif->data->fs_shift, fs << memif->data->fs_shift);
  return 0;
}