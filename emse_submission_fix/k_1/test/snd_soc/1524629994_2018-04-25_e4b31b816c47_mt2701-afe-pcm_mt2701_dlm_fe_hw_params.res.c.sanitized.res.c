static int mt2701_dlm_fe_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
  int channels = params_channels(params);
  regmap_update_bits(afe->regmap, AFE_MEMIF_PBUF_SIZE, AFE_MEMIF_PBUF_SIZE_DLM_MASK, AFE_MEMIF_PBUF_SIZE_FULL_INTERLEAVE);
  regmap_update_bits(afe->regmap, AFE_MEMIF_PBUF_SIZE, AFE_MEMIF_PBUF_SIZE_DLM_BYTE_MASK, AFE_MEMIF_PBUF_SIZE_DLM_32BYTES);
  regmap_update_bits(afe->regmap, AFE_MEMIF_PBUF_SIZE, AFE_MEMIF_PBUF_SIZE_DLM_CH_MASK, AFE_MEMIF_PBUF_SIZE_DLM_CH(channels));
  return mtk_afe_fe_hw_params(substream, params, dai);
}