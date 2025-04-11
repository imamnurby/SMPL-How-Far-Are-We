@@
type T;
identifier component_var, dai_var, rtd_var;
identifier new_afe_var;
@@
 T *rtd_var = substream->private_data;
- struct snd_soc_component *component_var =
-   snd_soc_rtdcom_lookup(rtd_var, ...);
- T *new_afe_var = snd_soc_component_get_drvdata(component_var);
+ T *new_afe_var = snd_soc_dai_get_drvdata(dai_var);
...