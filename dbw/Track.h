#pragma once
#include <memory>
#include <string>
#include <vector>
#include <clap/clap.h>
#include "ProcessBuffer.h"

class ProcessBuffer;
class Composer;
class Line;
class Module;

class Track {
public:
    Track(std::string name, Composer* composer);
    ~Track();
    void process(const ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime);
    void render();
    void renderLine(int line);
    void changeMaxLine(int value);
    void addModule(std::string path, uint32_t index);
    ProcessBuffer _processBuffer;
    float* _out = nullptr;

    std::string _name;
    size_t _ncolumns;
    std::vector<std::unique_ptr<Line>> _lines;
    std::vector<std::unique_ptr<Module>> _modules;
    int16_t _lastKey;

    std::string _pluginPath = { "C:\\Program Files\\Common Files\\CLAP\\Surge Synth Team\\Surge XT.clap" };
    Composer* _composer;
    bool _openModuleSelector = false;
};

