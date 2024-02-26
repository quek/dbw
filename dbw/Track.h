#pragma once
#include <memory>
#include <string>
#include <vector>
#include <clap/clap.h>
#include "GainModule.h"
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
    void addModule(std::string path, uint32_t index);
    void addModule(Module* module);
    void addLane(Lane* lane);
    bool isAvailableSidechainSrc(Track* dst);
    virtual nlohmann::json toJson() override;
    void addTrack();
    void addTrack(Track* track);
    void addTrack(std::unique_ptr<Track> track);
    std::vector<std::unique_ptr<Track>> deleteTracks(std::vector<Track*> tracks);
    std::vector<std::unique_ptr<Track>>::iterator findTrack(Track* track);
    void insertTrack(std::vector<std::unique_ptr<Track>>::iterator it, std::unique_ptr<Track>& track);
    void insertTracks(std::vector<std::unique_ptr<Track>>::iterator it, std::vector<std::unique_ptr<Track>>& tracks);
    bool isMasterTrack();
    Track* getParent();
    void setParent(Track* parent);
    void resolveModuleReference();
    void play(Scene* scene);
    void stop(Scene* scene);
    bool isAllLanesPlaying(Scene* scene);
    bool isAllLanesStoped(Scene* scene);
    void allTracks(std::vector<Track*>& tracks);
    std::vector<Module*> allModules();
    const std::vector<std::unique_ptr<Track>>& getTracks();
    std::vector<std::unique_ptr<Track>>::iterator tracksBegin();
    std::vector<std::unique_ptr<Track>>::iterator tracksEnd();

    std::unique_ptr<GainModule> _gain;
    std::unique_ptr<Fader> _fader;
    ProcessBuffer _processBuffer;

    std::vector<std::unique_ptr<Lane>> _lanes;
    std::vector<std::unique_ptr<Module>> _modules;

    bool _openModuleSelector = false;
    float _width = 150.0f;

    bool _showTracks = true;

private:
    std::unique_ptr<Track> deleteTrack(Track* track);

    Track* _parent = nullptr;
    Composer* _composer = nullptr;
    std::vector<std::unique_ptr<Track>> _tracks;
};
