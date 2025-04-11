static int mt2701_btmrg_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
  int stream_fs;
  u32 val, msk;
  stream_fs = params_rate(params);
  if ((stream_fs != 8000) && (stream_fs != 16000))
    {
      dev_err(afe->dev, "%s() btmgr not support this stream_fs %d\n", __func__, stream_fs);
      return -EINVAL;
    }
  regmap_update_bits(afe->regmap, AFE_MRGIF_CON, AFE_MRGIF_CON_I2S_MODE_MASK, AFE_MRGIF_CON_I2S_MODE_32K);
  val = AFE_DAIBT_CON0_BT_FUNC_EN | AFE_DAIBT_CON0_BT_FUNC_RDY | AFE_DAIBT_CON0_MRG_USE;
  msk = val;
  if (stream_fs == 16000)
    val |= AFE_DAIBT_CON0_BT_WIDE_MODE_EN;
  msk |= AFE_DAIBT_CON0_BT_WIDE_MODE_EN;
  regmap_update_bits(afe->regmap, AFE_DAIBT_CON0, msk, val);
  regmap_update_bits(afe->regmap, AFE_DAIBT_CON0, AFE_DAIBT_CON0_DAIBT_EN, AFE_DAIBT_CON0_DAIBT_EN);
  regmap_update_bits(afe->regmap, AFE_MRGIF_CON, AFE_MRGIF_CON_MRG_I2S_EN, AFE_MRGIF_CON_MRG_I2S_EN);
  regmap_update_bits(afe->regmap, AFE_MRGIF_CON, AFE_MRGIF_CON_MRG_EN, AFE_MRGIF_CON_MRG_EN);
  return 0;
}