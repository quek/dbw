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
    virtual ~Track();
    virtual void process(const ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime);
    virtual void render();
    virtual void renderLine(int line);
    void changeMaxLine(int value);
    void addModule(std::string path, uint32_t index);
    ProcessBuffer _processBuffer;
    float* _out = nullptr;

    std::string _name;
    size_t _ncolumns;
    std::vector<std::unique_ptr<Line>> _lines;
    std::vector<std::unique_ptr<Module>> _modules;
    std::vector<int16_t> _lastKeys;

    Composer* _composer;
    bool _openModuleSelector = false;
};

