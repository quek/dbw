#pragma once
#include <memory>
#include <string>
#include <vector>
#include <clap/clap.h>
#include "ProcessBuffer.h"
#include "XMLMixin.h"

class ProcessBuffer;
class Composer;
class Fader;
class Lane;
class Module;

class Track : public XMLMixin {
public:
    Track(std::string name, Composer* composer);
    virtual ~Track();
    void prepare(unsigned long framesPerBuffer);
    bool process(int64_t steadyTime);
    void render();
    void addModule(std::string path, uint32_t index);
    bool isAvailableSidechainSrc(Track* dst);
    uint32_t computeLatency();
    void doDCP();
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;
    static std::unique_ptr<Track> fromXml(tinyxml2::XMLElement* element, Composer* composer);

    std::unique_ptr<Fader> _fader;
    ProcessBuffer _processBuffer;
    bool _processed = false;

    std::string _name;
    std::vector<std::unique_ptr<Lane>> _lanes;
    std::vector<std::unique_ptr<Module>> _modules;
    Module* _waitingModule = nullptr;
    uint32_t _latency = 0;

    Composer* _composer;
    bool _openModuleSelector = false;

private:
    virtual const char* role() { return "regular"; }
};
