#pragma once
#include <string>
#include <clap/clap.h>
#include "tinyxml2/tinyxml2.h"
#include "ProcessBuffer.h"

class Track;

class Module {
public:
    Module(std::string name, Track* track) : _name(name), _track(track) {}
    virtual ~Module();
    virtual void openGui() { _didOpenGui = true; };
    virtual void closeGui() { _didOpenGui = false; };
    virtual void start() {};
    virtual void stop() {};
    virtual void render();
    virtual bool process(ProcessBuffer* /*buffer*/, int64_t /*steadyTime*/) { return true; }
    virtual tinyxml2::XMLElement* dawProject(tinyxml2::XMLDocument* doc) = 0;

    Track* _track;
    std::string _name;
    bool _didOpenGui = false;
};
