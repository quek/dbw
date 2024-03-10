#pragma once
#include <filesystem>
#include <string>
#include <clap/clap.h>
#include <nlohmann/json.hpp>
#include "Connection.h"
#include "Nameable.h"
#include "ProcessBuffer.h"
#include "Neko.h"
#include "Param.h"

class Track;

class Module : public Nameable {
public:
    Module(const nlohmann::json& json);
    Module(std::string name, Track* track) : Nameable(name), _track(track) {}
    virtual ~Module();
    virtual void openGui() { _didOpenGui = true; }
    virtual void closeGui() { _didOpenGui = false; }
    virtual void start();
    virtual bool isStarting() { return _isStarting; }
    virtual void stop() { _isStarting = false; }
    virtual void render(float width = 0.0f, float height = 0.0f);
    virtual void renderContent() {}
    virtual bool process(ProcessBuffer* buffer, int64_t steadyTime);
    void processConnections();
    virtual void onResize(int /*width*/, int /*height*/) {}
    virtual void loadState(std::filesystem::path /*path*/) {}
    virtual void connect(Module* from, int outputIndex, int inputIndex);
    int nbuses() const;
    ProcessBuffer& getProcessBuffer();
    virtual uint32_t getComputedLatency();
    virtual void setComputedLatency(uint32_t computedLatency);
    std::unique_ptr<Param>& getParam(uint32_t paramId);
    virtual void addParameterChange(Param*, int32_t sampleOffset);
    virtual void addParameterChange(Param*, int32_t, double) {};
    virtual void updateEditedParamIdList(ParamId id);

    static Module* create(std::string& type, std::string& id);
    static Module* fromJson(const nlohmann::json&);
    virtual nlohmann::json toJson() override;

    Track* _track = nullptr;
    bool _didOpenGui = false;
    std::vector<std::unique_ptr<Connection>> _connections;
    int _ninputs = 0;
    int _noutputs = 0;
    int _neventInputs = 0;
    int _neventOutputs = 0;
    uint32_t _latency = 0;
    uint32_t _computedLatency = 0;

protected:
    bool _isStarting = false;
    std::map<uint32_t, std::unique_ptr<Param>> _idParamMap;
    std::list<uint32_t> _editedParamIdList;
};
