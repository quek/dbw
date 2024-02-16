#pragma once
#include <filesystem>
#include <string>
#include <clap/clap.h>
#include "tinyxml2/tinyxml2.h"
#include "ProcessBuffer.h"

class Track;

class Module {
public:
    Module(std::string name, Track* track) : _name(name), _track(track) {}
    virtual ~Module();
    virtual void openGui() { _didOpenGui = true; }
    virtual void closeGui() { _didOpenGui = false; }
    virtual void start() { _isStarting = true;};
    virtual bool isStarting() { return _isStarting; }
    virtual void stop() { _isStarting = false; }
    virtual void render();
    virtual void renderContent() {}
    virtual bool process(ProcessBuffer* /*buffer*/, int64_t /*steadyTime*/) { return true; }
    virtual void onResize(int /*width*/, int /*height*/) {}
    virtual void loadState(std::filesystem::path /*path*/) {}
    virtual tinyxml2::XMLElement* dawProject(tinyxml2::XMLDocument* doc) = 0;

    Track* _track;
    std::string _name;
    bool _didOpenGui = false;


protected:
    bool _isStarting = false;
    int _ninputs = 0;
    int _noutputs = 0;
    int _neventInputs = 0;
    int _neventOutputs = 0;
};
