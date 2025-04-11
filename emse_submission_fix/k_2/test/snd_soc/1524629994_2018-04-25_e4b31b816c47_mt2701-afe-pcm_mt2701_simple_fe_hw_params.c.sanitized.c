static int mt2701_simple_fe_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
  int stream_dir = substream->stream;

  if (stream_dir == SNDRV_PCM_STREAM_PLAYBACK)
    {
      regmap_update_bits(afe->regmap, AFE_MEMIF_PBUF_SIZE, AFE_MEMIF_PBUF_SIZE_DLM_MASK, AFE_MEMIF_PBUF_SIZE_PAIR_INTERLEAVE);
    }
  return mtk_afe_fe_hw_params(substream, params, dai);
}