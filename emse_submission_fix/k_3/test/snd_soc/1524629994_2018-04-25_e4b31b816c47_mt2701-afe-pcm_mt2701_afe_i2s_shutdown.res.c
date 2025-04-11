static void mt2701_afe_i2s_shutdown(struct snd_pcm_substream *substream,
				    struct snd_soc_dai *dai)
{
	struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mt2701_afe_private *afe_priv = afe->platform_priv;
	int i2s_num = mt2701_dai_num_to_i2s(afe, dai->id);
	struct mt2701_i2s_path *i2s_path;
	if (i2s_num < 0)
		return;
	i2s_path = &afe_priv->i2s_path[i2s_num];
	if (i2s_path->occupied[substream->stream])
		i2s_path->occupied[substream->stream] = 0;
	else
		goto I2S_UNSTART;
	mt2701_afe_i2s_path_shutdown(substream, dai, i2s_num, 0);
	/* need to disable i2s-out path when disable i2s-in */
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		mt2701_afe_i2s_path_shutdown(substream, dai, i2s_num, 1);
I2S_UNSTART:
	/* disable mclk */
	mt2701_afe_disable_mclk(afe, i2s_num);
}