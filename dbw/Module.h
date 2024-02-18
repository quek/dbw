#pragma once
#include <filesystem>
#include <string>
#include <clap/clap.h>
#include "tinyxml2/tinyxml2.h"
#include "Connection.h"
#include "ProcessBuffer.h"
#include "XMLMixin.h"

class Track;

class Module :public XMLMixin {
public:
    Module(std::string name, Track* track) : _name(name), _track(track) {}
    virtual ~Module();
    virtual void openGui() { _didOpenGui = true; }
    virtual void closeGui() { _didOpenGui = false; }
    virtual void start();
    virtual bool isStarting() { return _isStarting; }
    virtual void stop() { _isStarting = false; }
    virtual void render();
    virtual void renderContent() {}
    virtual bool isWaitingFrom();
    virtual bool isWaitingTo();
    virtual bool process(ProcessBuffer* buffer, int64_t steadyTime);
    virtual void onResize(int /*width*/, int /*height*/) {}
    virtual void loadState(std::filesystem::path /*path*/) {}
    virtual void prepare();
    virtual void connect(Module* from, int outputIndex, int inputIndex);
    ProcessBuffer& getProcessBuffer();
    virtual tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc);

    Track* _track;
    std::string _name;
    bool _didOpenGui = false;
    std::vector<std::unique_ptr<Connection>> _connections;
    bool _processed = false;

protected:
    bool _isStarting = false;
    int _ninputs = 0;
    int _noutputs = 0;
    int _neventInputs = 0;
    int _neventOutputs = 0;

};
