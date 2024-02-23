#pragma once
#include <vector>
#include <portaudio.h>

class App;

class AudioEngineWindow {
public:
    AudioEngineWindow(App* app);
    void render();
    void show();

private:
    App* _app;
    bool _show = false;
    std::vector<const PaHostApiInfo*> _apiInfos;
    std::vector<const PaDeviceInfo*> _deviceInfos;
    std::vector<std::vector<double>> _supportedStandardSampleRates;

    int _deviceIndex = 0;
    double _sampleRate = 48000.0;
    unsigned long _bufferSize = 1024;
};

