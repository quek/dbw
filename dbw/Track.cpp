#include "Track.h"
#include <algorithm>
#include <mutex>
#include <ranges>
#include "imgui.h"
#include "App.h"
#include "AudioEngine.h"
#include "Clip.h"
#include "Command.h"
#include "Composer.h"
#include "Config.h"
#include "Fader.h"
#include "Midi.h"
#include "Module.h"
#include "ClapModule.h"
#include "ClapHost.h"
#include "Lane.h"
#include "command/AddModule.h"

Track::Track(const nlohmann::json& json, SerializeContext& context) : Nameable(json, context)
{
    _width = json["_width"];
    for (const auto& x : json["_lanes"])
    {
        addLane(new Lane(x, context));
    }

    _gain.reset(new GainModule(json["_gain"], context));
    _gain->_track = this;
    _gain->start();

    _fader.reset(new Fader(json["_fader"], context));
    _fader->_track = this;
    _fader->start();

    if (json.contains("_modules"))
    {
        for (const auto& x : json["_modules"])
        {
            addModule(gPluginManager.create(x, context));
        }
    }

    if (json.contains("_tracks"))
    {
        for (const auto& x : json["_tracks"])
        {
            Track* track = new Track(x, context);
            track->_parent = this;
            _tracks.emplace_back(track);
        }
    }
}

Track::Track(std::string name, Composer* composer) :
    Nameable(name),
    _gain(new GainModule("Gain", this)),
    _fader(new Fader("Fader", this)),
    _composer(composer)
{
    addLane(new Lane());
    _gain->start();
    _fader->start();
}

Track::~Track()
{
}

Composer* Track::getComposer()
{
    if (_composer)
    {
        return _composer;
    }
    if (_parent)
    {
        return _parent->getComposer();
    }
    return nullptr;
}

void Track::setComposer(Composer* composer)
{
    _composer = composer;
}

void Track::prepare(unsigned long framesPerBuffer)
{
    _processBuffer.clear();
    int nbuses = 1;
    for (auto& module : _modules)
    {
        if (nbuses < module->_ninputs)
        {
            nbuses = module->_ninputs;
        }
        if (nbuses < module->_noutputs)
        {
            nbuses = module->_noutputs;
        }
    }
    _processBuffer.ensure(framesPerBuffer, nbuses, 2);

    if (isSelected())
    {
        for (auto& midiDevice : getComposer()->app()->getMidiInDevices())
        {
            for (uint32_t i = 0; i < midiDevice->getEventList().size(midiDevice->getEventList().clapInputEvents()); ++i)
            {
                _processBuffer._eventOut.push(midiDevice->getEventList().get(midiDevice->getEventList().clapInputEvents(), i));
            }
        }
    }

    for (const auto& track : _tracks)
    {
        track->prepare(framesPerBuffer);
    }
}

void Track::prepareEvent()
{
    double oneBeatSec = 60.0 / getComposer()->_bpm;
    Composer* composer = getComposer();
    double begin = composer->_playTime;
    double end = composer->_nextPlayTime;
    double loopBegin = composer->_loopStartTime;
    double loopEnd = composer->_loopEndTime;
    for (auto& lane : _lanes)
    {
        lane->prepareProcessBuffer(begin, end, loopBegin, loopEnd, oneBeatSec);
    }

    getComposer()->_sceneMatrix->process(this);

    for (const auto& track : _tracks)
    {
        track->prepareEvent();
    }
}

void Track::addModule(std::string path, uint32_t index)
{
    ClapHost* pluginHost = new ClapHost(this);
    pluginHost->load(path.c_str(), index);
    Module* module = new ClapModule(pluginHost->_name, this, pluginHost);
    // TODO
    _modules.emplace_back(module);
    module->start();
    module->openGui();
    getComposer()->computeProcessOrder();
}

void Track::addModule(Module* module)
{
    module->_track = this;
    _modules.emplace_back(module);
    module->start();
    if (getComposer())
    {
        getComposer()->computeProcessOrder();
    }
}

void Track::addLane(Lane* lane)
{
    lane->_track = this;
    _lanes.emplace_back(lane);
}

bool Track::isAvailableSidechainSrc(Track* dst)
{
    if (this == dst)
    {
        // 当該モジュールより前のは使える
        return true;
    }
    // TODO
    return true;
}

nlohmann::json Track::toJson(SerializeContext& context)
{
    nlohmann::json json = Nameable::toJson(context);
    json["type"] = TYPE;
    json["_width"] = _width;
    json["_gain"].update(_gain->toJson(context));
    json["_fader"].update(_fader->toJson(context));

    nlohmann::json modules = nlohmann::json::array();
    for (auto& module : _modules)
    {
        modules.emplace_back(module->toJson(context));
    }
    json["_modules"] = modules;

    nlohmann::json lanes = nlohmann::json::array();
    for (auto& lane : _lanes)
    {
        lanes.emplace_back(lane->toJson(context));
    }
    json["_lanes"] = lanes;

    nlohmann::json tracks = nlohmann::json::array();
    for (auto& track : _tracks)
    {
        tracks.emplace_back(track->toJson(context));
    }
    json["_tracks"] = tracks;

    return json;
}

void Track::addTrack()
{
    std::string name = std::to_string(_tracks.size() + 1);
    Track* track = new Track(name);
    addTrack(std::unique_ptr<Track>(track));
}

void Track::addTrack(Track* track)
{
    addTrack(std::unique_ptr<Track>(track));
}

void Track::addTrack(std::unique_ptr<Track> track)
{
    insertTrack(_tracks.end(), track);
}

std::unique_ptr<Track> Track::deleteTrack(Track* track)
{
    for (auto it = _tracks.begin(); it != _tracks.end(); ++it)
    {
        if ((*it).get() == track)
        {
            std::unique_ptr<Track> ptr(std::move(*it));
            _tracks.erase(it);
            return ptr;
        }
        std::unique_ptr<Track> ptr = (*it)->deleteTrack(track);
        if (ptr)
        {
            return ptr;
        }
    }
    return nullptr;
}

// TODO MasterTrack 専用なので MasterTrack クラス作るべき？
std::vector<std::unique_ptr<Track>> Track::deleteTracks(std::vector<Track*> tracks)
{
    std::vector<std::unique_ptr<Track>> deletedTracks;
    std::vector<Track*> allDeletedTracks;
    for (const auto& track : tracks)
    {
        auto x = deleteTrack(track);
        std::vector<Track*> includeChildren;
        x->allTracks(includeChildren);
        allDeletedTracks.insert(allDeletedTracks.end(), includeChildren.begin(), includeChildren.end());
        deletedTracks.emplace_back(std::move(x));
    }
    for (auto& track : deletedTracks)
    {
        for (auto& module : track->allModules())
        {
            std::vector<Connection*> willDeleteConnections;
            for (auto& connection : module->_connections)
            {
                if (std::ranges::all_of(allDeletedTracks, [&connection](auto& deletedTrack)
                {
                    return connection->_from->_track != deletedTrack;
                }) ||
                    std::ranges::all_of(allDeletedTracks, [&connection](auto& deletedTrack)
                {
                    return connection->_to->_track != deletedTrack;
                }))
                {
                    willDeleteConnections.push_back(connection.get());
                }
            }
            for (auto& connection : willDeleteConnections)
            {
                if (connection->_to == module)
                {
                    std::erase_if(connection->_from->_connections, [module](auto& x) { return x->_to == module; });
                }
                else
                {
                    std::erase_if(connection->_to->_connections, [module](auto& x) { return x->_from == module; });
                }
                std::erase_if(module->_connections, [connection](auto& x) { return x.get() == connection; });
            }
        }

    }
    return deletedTracks;
}

void Track::insertTrack(std::vector<std::unique_ptr<Track>>::iterator it, std::unique_ptr<Track>& track)
{
    track->_parent = this;
    this->_gain->connect(track->_fader.get(), 0, 0);
    _tracks.insert(it, std::move(track));
}

void Track::insertTracks(std::vector<std::unique_ptr<Track>>::iterator it, std::vector<std::unique_ptr<Track>>& tracks)
{
    for (auto& track : tracks | std::views::reverse)
    {
        Track* p = track.get();
        insertTrack(it, track);
        it = findTrack(p);
    }
}

void Track::insertTracksAfterThis(std::vector<std::unique_ptr<Track>>& tracks)
{
    auto at = getAt() + 1;
    getParent()->insertTracks(at, tracks);
}

void Track::insertTracksBeforeThis(std::vector<std::unique_ptr<Track>>& tracks)
{
    auto at = getAt();
    getParent()->insertTracks(at, tracks);
}

bool Track::isMasterTrack()
{
    return false;
}

std::vector<std::unique_ptr<Track>>::iterator Track::getAt()
{
    return _parent->findTrack(this);
}

MasterTrack* Track::getMasterTrack()
{
    return _parent->getMasterTrack();
}

Track* Track::getParent()
{
    return _parent;
}

void Track::setParent(Track* parent)
{
    _parent = parent;
}

void Track::resolveModuleReference()
{
    for (const auto& module : allModules())
    {
        for (const auto& connection : module->_connections)
        {
            connection->resolveModuleReference();
        }
    }
    for (const auto& track : _tracks)
    {
        track->resolveModuleReference();
    }
}

void Track::play(Scene* scene)
{
    for (const auto& lane : _lanes)
    {
        auto& clipSlot = lane->getClipSlot(scene);
        clipSlot->play();
    }
    for (const auto& track : _tracks)
    {
        track->play(scene);
    }
}

void Track::stop(Scene* scene)
{
    for (const auto& lane : _lanes)
    {
        auto& clipSlot = lane->getClipSlot(scene);
        clipSlot->stop();
    }
    for (const auto& track : _tracks)
    {
        track->stop(scene);
    }
}

bool Track::isAllLanesPlaying(Scene* scene)
{
    for (const auto& lane : _lanes)
    {
        auto& clipSlot = lane->getClipSlot(scene);
        if (!clipSlot->_playing)
        {
            return false;
        }
    }
    for (const auto& track : _tracks)
    {
        if (!track->isAllLanesPlaying(scene))
        {
            return false;
        }
    }
    return true;
}

bool Track::isAllLanesStoped(Scene* scene)
{
    for (const auto& lane : _lanes)
    {
        auto& clipSlot = lane->getClipSlot(scene);
        if (clipSlot->_playing)
        {
            return false;
        }
    }
    for (const auto& track : _tracks)
    {
        if (track->isAllLanesPlaying(scene))
        {
            return false;
        }
    }
    return true;
}

bool Track::isSelected()
{
    Composer* composer = getComposer();
    if (!composer)
    {
        return false;
    }
    return composer->selectedTracksContain(this);
}

void Track::allTracks(std::vector<Track*>& tracks)
{
    tracks.push_back(this);
    for (const auto& x : _tracks)
    {
        x->allTracks(tracks);
    }
}

std::vector<Module*> Track::allModules()
{
    std::vector<Module*> vec{ _gain.get() };
    for (const auto& module : _modules)
    {
        vec.push_back(module.get());
    }
    vec.push_back(_fader.get());
    return vec;
}

std::vector<std::unique_ptr<Track>>::iterator Track::findTrack(Track* track)
{
    return std::ranges::find_if(_tracks, [track](const auto& x) { return x.get() == track; });
}

bool Track::included(std::vector<Track*>& tracks)
{
    for (auto& track : tracks)
    {
        if (track == this)
        {
            return true;
        }
        if (included(track->_tracks))
        {
            return true;
        }
    }
    return false;
}

bool Track::included(std::vector<std::unique_ptr<Track>>& tracks)
{
    for (auto& track : tracks)
    {
        if (track.get() == this)
        {
            return true;
        }
        if (included(track->_tracks))
        {
            return true;
        }
    }
    return false;
}

const std::vector<std::unique_ptr<Track>>& Track::getTracks()
{
    return _tracks;
}

std::vector<std::unique_ptr<Track>>::iterator Track::tracksBegin()
{
    return _tracks.begin();
}

std::vector<std::unique_ptr<Track>>::iterator Track::tracksEnd()
{
    return _tracks.end();
}

