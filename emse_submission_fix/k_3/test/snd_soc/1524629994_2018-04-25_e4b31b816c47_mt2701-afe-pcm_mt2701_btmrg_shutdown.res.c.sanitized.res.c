static void mt2701_btmrg_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
  struct mt2701_afe_private *afe_priv = afe->platform_priv;

  if (!afe_priv->mrg_enable[!substream->stream])
    {
      regmap_update_bits(afe->regmap, AFE_DAIBT_CON0, AFE_DAIBT_CON0_DAIBT_EN, 0);
      regmap_update_bits(afe->regmap, AFE_MRGIF_CON, AFE_MRGIF_CON_MRG_EN, 0);
      regmap_update_bits(afe->regmap, AFE_MRGIF_CON, AFE_MRGIF_CON_MRG_I2S_EN, 0);
      mt2701_disable_btmrg_clk(afe);
    }
  afe_priv->mrg_enable[substream->stream] = 0;
}