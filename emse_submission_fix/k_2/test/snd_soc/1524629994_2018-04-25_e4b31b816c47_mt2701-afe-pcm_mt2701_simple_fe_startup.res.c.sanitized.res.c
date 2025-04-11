static int mt2701_simple_fe_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
  int stream_dir = substream->stream;
  int memif_num = rtd->cpu_dai->id;
  struct mtk_base_afe_memif *memif_tmp;

  if (stream_dir == SNDRV_PCM_STREAM_PLAYBACK)
    {
      memif_tmp = &afe->memif[MT2701_MEMIF_DLM];
      if (memif_tmp->substream)
        {
          dev_warn(afe->dev, "%s memif is not available, stream_dir %d, memif_num %d\n", __func__, stream_dir, memif_num);
          return -EBUSY;
        }
    }
  return mtk_afe_fe_startup(substream, dai);
}