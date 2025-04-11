@@
expression rtd, name;
expression substream, dai;
identifier component, drvdata;
@@
- rtd = substream->private_data;
- component = snd_soc_rtdcom_lookup(rtd, name);
- drvdata = snd_soc_component_get_drvdata(component);
+ drvdata = snd_soc_dai_get_drvdata(dai);