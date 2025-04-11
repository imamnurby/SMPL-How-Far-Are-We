int mtk_afe_fe_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
  struct mtk_base_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
  int hd_audio = 0;

  switch (substream->runtime->format)
    {
    case SNDRV_PCM_FORMAT_S16_LE:
      hd_audio = 0;
      break;
    case SNDRV_PCM_FORMAT_S32_LE:
      hd_audio = 1;
      break;
    case SNDRV_PCM_FORMAT_S24_LE:
      hd_audio = 1;
      break;
    default:
      dev_err(afe->dev, "%s() error: unsupported format %d\n", __func__, substream->runtime->format);
      break;
    }
  mtk_regmap_update_bits(afe->regmap, memif->data->hd_reg, 1 << memif->data->hd_shift, hd_audio << memif->data->hd_shift);
  return 0;
}