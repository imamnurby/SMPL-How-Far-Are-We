static int mt2701_i2s_path_prepare_enable(struct snd_pcm_substream *substream,
					  struct snd_soc_dai *dai,
					  int i2s_num,
					  int dir_invert)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
	struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
	struct mt2701_afe_private *afe_priv = afe->platform_priv;
	struct mt2701_i2s_path *i2s_path = &afe_priv->i2s_path[i2s_num];
	const struct mt2701_i2s_data *i2s_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	int reg, fs, w_len = 1; /* now we support bck 64bits only */
	int stream_dir = substream->stream;
	unsigned int mask = 0, val = 0;
	if (dir_invert) {
		if (stream_dir == SNDRV_PCM_STREAM_PLAYBACK)
			stream_dir = SNDRV_PCM_STREAM_CAPTURE;
		else
			stream_dir = SNDRV_PCM_STREAM_PLAYBACK;
	}
	i2s_data = i2s_path->i2s_data[stream_dir];
	/* no need to enable if already done */
	i2s_path->on[stream_dir]++;
	if (i2s_path->on[stream_dir] != 1)
		return 0;
	fs = mt2701_afe_i2s_fs(runtime->rate);
	mask = ASYS_I2S_CON_FS |
	       ASYS_I2S_CON_I2S_COUPLE_MODE | /* 0 */
	       ASYS_I2S_CON_I2S_MODE |
	       ASYS_I2S_CON_WIDE_MODE;
	val = ASYS_I2S_CON_FS_SET(fs) |
	      ASYS_I2S_CON_I2S_MODE |
	      ASYS_I2S_CON_WIDE_MODE_SET(w_len);
	if (stream_dir == SNDRV_PCM_STREAM_CAPTURE) {
		mask |= ASYS_I2S_IN_PHASE_FIX;
		val |= ASYS_I2S_IN_PHASE_FIX;
	}
	regmap_update_bits(afe->regmap, i2s_data->i2s_ctrl_reg, mask, val);
	if (stream_dir == SNDRV_PCM_STREAM_PLAYBACK)
		reg = ASMO_TIMING_CON1;
	else
		reg = ASMI_TIMING_CON1;
	regmap_update_bits(afe->regmap, reg,
			   i2s_data->i2s_asrc_fs_mask
			   << i2s_data->i2s_asrc_fs_shift,
			   fs << i2s_data->i2s_asrc_fs_shift);
	/* enable i2s */
	mt2701_afe_enable_i2s(afe, i2s_num, stream_dir);
	/* reset i2s hw status before enable */
	regmap_update_bits(afe->regmap, i2s_data->i2s_ctrl_reg,
			   ASYS_I2S_CON_RESET, ASYS_I2S_CON_RESET);
	udelay(1);
	regmap_update_bits(afe->regmap, i2s_data->i2s_ctrl_reg,
			   ASYS_I2S_CON_RESET, 0);
	udelay(1);
	regmap_update_bits(afe->regmap, i2s_data->i2s_ctrl_reg,
			   ASYS_I2S_CON_I2S_EN, ASYS_I2S_CON_I2S_EN);
	return 0;
}