@@
type T1, T2;
identifier substream;
identifier dai, rtd, component, afe;
constant AFE_PCM_NAME;
@@
-  T1 *rtd = substream->private_data;
-  T2 *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
-  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
+  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);