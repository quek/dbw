# VST3 Sdk

https://steinbergmedia.github.io/vst3_dev_portal/pages/Tutorials/Building+the+examples/Building+the+examples+included+in+the+SDK+Windows.html
https://qiita.com/hotwatermorning/items/adf71e8a5b1a701ca585

```
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ../vst3sdk -DSMTG_CREATE_PLUGIN_LINK=0 -DSMTG_ADD_VST3_HOSTING_SAMPLES=OFF -DSMTG_ADD_VST3_PLUGINS_SAMPLES=OFF -DSMTG_ADD_VSTGUI=OFF
cmake --build . --config Release
```

