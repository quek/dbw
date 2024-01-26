#pragma once
#include <string>
#include <clap/clap.h>
#include "ProcessBuffer.h"

class Track;

class Module {
public:
    Module(std::string name, Track* track) : _name(name), _track(track) {}
    virtual ~Module() = default;
    virtual void openGui() { _didOpenGui = true;  };
    virtual void closeGui() { _didOpenGui = false; };
    virtual void start() {};
    virtual void stop() {};
    virtual void render();
    virtual void process(ProcessBuffer* in, unsigned long framesPerBuffer, int64_t steadyTime);
    ProcessBuffer _processBuffer;
    Track* _track;
    std::string _name;
    bool _didOpenGui = false;
};
