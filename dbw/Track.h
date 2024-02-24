#pragma once
#include <memory>
#include <string>
#include <vector>
#include <clap/clap.h>
#include "ProcessBuffer.h"
#include "Nameable.h"

class ProcessBuffer;
class Composer;
class Fader;
class Lane;
class Module;

class Track : public Nameable {
public:
    inline static const char* TYPE = "track";
    Track(const nlohmann::json& json);
    Track(std::string name, Composer* composer);
    virtual ~Track();
    void prepare(unsigned long framesPerBuffer);
    void prepareEvent();
    void render();
    void addModule(std::string path, uint32_t index);
    void addModule(Module* module);
    void addLane(Lane* lane);
    void addChild(Track* child);
    bool isAvailableSidechainSrc(Track* dst);
    uint32_t computeLatency();
    void doDCP();
    virtual nlohmann::json toJson() override;

    std::unique_ptr<Fader> _fader;
    ProcessBuffer _processBuffer;

    std::vector<std::unique_ptr<Lane>> _lanes;
    std::vector<std::unique_ptr<Module>> _modules;
    
    uint32_t _latency = 0;

    Composer* _composer = nullptr;
    bool _openModuleSelector = false;
    float _width = 150.0f;

private:
    Track* _parent = nullptr;
    std::vector<std::unique_ptr<Track>> _children;
};
