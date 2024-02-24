#pragma once
#include <memory>
#include <string>
#include <vector>
#include <clap/clap.h>
#include "ProcessBuffer.h"
#include "TracksHolder.h"

class ProcessBuffer;
class Composer;
class Fader;
class Lane;
class Module;

class Track : public TracksHolder {
public:
    inline static const char* TYPE = "track";
    Track(const nlohmann::json& json);
    Track(const std::string& name);
    virtual ~Track();
    Composer* getComposer() override;
    void prepare(unsigned long framesPerBuffer);
    void prepareEvent();
    void render();
    void addModule(std::string path, uint32_t index);
    void addModule(Module* module);
    void addLane(Lane* lane);
    bool isAvailableSidechainSrc(Track* dst);
    uint32_t computeLatency();
    void doDCP();
    TracksHolder* getTracksHolder();
    void setTracksHolder(TracksHolder* tracksHolder);
    virtual nlohmann::json toJson() override;

    std::unique_ptr<Fader> _fader;
    ProcessBuffer _processBuffer;

    std::vector<std::unique_ptr<Lane>> _lanes;
    std::vector<std::unique_ptr<Module>> _modules;

    uint32_t _latency = 0;

    bool _openModuleSelector = false;
    float _width = 150.0f;

private:
    TracksHolder* _tracksHolder = nullptr;
};
