@@
type T;
identifier AFE;
parameter P;
expression OLD_RTD_LOOKUP;
@@
- struct snd_soc_component *component = snd_soc_rtdcom_lookup(OLD_RTD_LOOKUP, ...);
- T AFE = snd_soc_component_get_drvdata(component);
+ T AFE = snd_soc_dai_get_drvdata(P);