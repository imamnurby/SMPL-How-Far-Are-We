@@
expression substream, dai, rtd;
identifier component;
@@
- rtd = substream->private_data;
- component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
- struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
+ struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
@@
expression substream, dai, rtd;
identifier component;
expression const_name;
@@
- rtd = substream->private_data;
- component = snd_soc_rtdcom_lookup(rtd, const_name);
- struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
+ struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);
@@
expression substream, dai;
identifier component;
@@
  struct snd_soc_pcm_runtime *rtd = substream->private_data;
- struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
- struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
+ struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);