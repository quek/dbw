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
    virtual void process(int64_t steadyTime);
    virtual void render();
    void addModule(std::string path, uint32_t index);
    bool isAvailableSidechainSrc(Track* dst);
    tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* doc) override;
    static std::unique_ptr<Track> fromXml(tinyxml2::XMLElement* element);

    std::unique_ptr<Fader> _fader;
    ProcessBuffer _processBuffer;

    std::string _name;
    std::vector<std::unique_ptr<Lane>> _trackLanes;
    std::vector<std::unique_ptr<Module>> _modules;

    Composer* _composer;
    bool _openModuleSelector = false;

private:
    virtual const char* role() { return "regular"; }
};
