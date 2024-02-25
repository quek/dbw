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
class Scene;

class Track : public Nameable {
public:
    inline static const char* TYPE = "track";
    Track(const nlohmann::json& json);
    Track(std::string name, Composer* composer = nullptr);
    virtual ~Track();
    Composer* getComposer();
    void setComposer(Composer* composer);
    void prepare(unsigned long framesPerBuffer);
    void prepareEvent();
    void render();
    void addModule(std::string path, uint32_t index);
    void addModule(Module* module);
    void addLane(Lane* lane);
    bool isAvailableSidechainSrc(Track* dst);
    uint32_t computeLatency();
    void doDCP();
    virtual nlohmann::json toJson() override;
    void addTrack();
    void addTrack(Track* track);
    void addTrack(std::unique_ptr<Track> track);
    Track* getParent();
    void setParent(Track* parent);
    void resolveModuleReference();
    void play(Scene* scene);
    void stop(Scene* scene);
    bool isAllLanesPlaying(Scene* scene);
    bool isAllLanesStoped(Scene* scene);
    void allTracks(std::vector<Track*>& tracks);

    std::unique_ptr<Fader> _fader;
    ProcessBuffer _processBuffer;

    std::vector<std::unique_ptr<Lane>> _lanes;
    std::vector<std::unique_ptr<Module>> _modules;

    uint32_t _latency = 0;

    bool _openModuleSelector = false;
    float _width = 150.0f;


    std::vector<std::unique_ptr<Track>>::iterator findTrack(Track* track);
    std::vector<std::unique_ptr<Track>>& getTracks();

    bool _showTracks = true;

private:
    Track* _parent = nullptr;
    Composer* _composer;
    std::vector<std::unique_ptr<Track>> _tracks;
};
