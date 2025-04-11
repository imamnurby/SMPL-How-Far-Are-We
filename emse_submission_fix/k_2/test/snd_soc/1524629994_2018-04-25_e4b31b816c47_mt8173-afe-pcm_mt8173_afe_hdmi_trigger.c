static int mt8173_afe_hdmi_trigger(struct snd_pcm_substream *substream, int cmd,
				   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
	struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
	dev_info(afe->dev, "%s cmd=%d %s\n", __func__, cmd, dai->name);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		regmap_update_bits(afe->regmap, AUDIO_TOP_CON0,
				   AUD_TCON0_PDN_HDMI | AUD_TCON0_PDN_SPDF, 0);
		/* set connections:  O30~O37: L/R/LS/RS/C/LFE/CH7/CH8 */
		regmap_write(afe->regmap, AFE_HDMI_CONN0,
				 AFE_HDMI_CONN0_O30_I30 |
				 AFE_HDMI_CONN0_O31_I31 |
				 AFE_HDMI_CONN0_O32_I34 |
				 AFE_HDMI_CONN0_O33_I35 |
				 AFE_HDMI_CONN0_O34_I32 |
				 AFE_HDMI_CONN0_O35_I33 |
				 AFE_HDMI_CONN0_O36_I36 |
				 AFE_HDMI_CONN0_O37_I37);
		/* enable Out control */
		regmap_update_bits(afe->regmap, AFE_HDMI_OUT_CON0, 0x1, 0x1);
		/* enable tdm */
		regmap_update_bits(afe->regmap, AFE_TDM_CON1, 0x1, 0x1);
		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		/* disable tdm */
		regmap_update_bits(afe->regmap, AFE_TDM_CON1, 0x1, 0);
		/* disable Out control */
		regmap_update_bits(afe->regmap, AFE_HDMI_OUT_CON0, 0x1, 0);
		regmap_update_bits(afe->regmap, AUDIO_TOP_CON0,
				   AUD_TCON0_PDN_HDMI | AUD_TCON0_PDN_SPDF,
				   AUD_TCON0_PDN_HDMI | AUD_TCON0_PDN_SPDF);
		return 0;
	default:
		return -EINVAL;
	}
}