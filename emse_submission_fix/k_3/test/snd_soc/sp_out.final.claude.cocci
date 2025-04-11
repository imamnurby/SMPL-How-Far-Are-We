@@
expression rtd, dai;
identifier afe;
@@
-  struct snd_soc_component *component = snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
-  struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
+  struct mtk_base_afe *afe = snd_soc_dai_get_drvdata(dai);


