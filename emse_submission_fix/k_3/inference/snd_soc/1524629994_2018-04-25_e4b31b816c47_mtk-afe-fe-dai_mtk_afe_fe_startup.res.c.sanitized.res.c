int mtk_afe_fe_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
  struct snd_pcm_runtime *runtime = substream->runtime;
  int memif_num = rtd->cpu_dai->id;
  struct mtk_base_afe_memif *memif = &afe->memif[memif_num];
  const struct snd_pcm_hardware *mtk_afe_hardware = afe->mtk_afe_hardware;
  int ret;
  memif->substream = substream;
  snd_pcm_hw_constraint_step(substream->runtime, 0, SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 16);

  mtk_regmap_update_bits(afe->regmap, memif->data->agent_disable_reg, 1 << memif->data->agent_disable_shift, 0 << memif->data->agent_disable_shift);
  snd_soc_set_runtime_hwparams(substream, mtk_afe_hardware);

  if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
    {
      int periods_max = mtk_afe_hardware->periods_max;
      ret = snd_pcm_hw_constraint_minmax(runtime, SNDRV_PCM_HW_PARAM_PERIODS, 3, periods_max);
      if (ret < 0)
        {
          dev_err(afe->dev, "hw_constraint_minmax failed\n");
          return ret;
        }
    }
  ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
  if (ret < 0)
    dev_err(afe->dev, "snd_pcm_hw_constraint_integer failed\n");

  if (memif->irq_usage < 0)
    {
      int irq_id = mtk_dynamic_irq_acquire(afe);
      if (irq_id != afe->irqs_size)
        {

          memif->irq_usage = irq_id;
        }
      else
        {
          dev_err(afe->dev, "%s() error: no more asys irq\n", __func__);
          ret = -EBUSY;
        }
    }
  return ret;
}