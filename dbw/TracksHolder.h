#pragma once
#include <memory>
#include <vector>
#include "Nameable.h"

class Composer;
class Track;

class TracksHolder : public Nameable {
public:
    TracksHolder(const nlohmann::json& json);
    TracksHolder(const std::string& name = "");
    virtual ~TracksHolder() = default;
    virtual void addTrack();
    virtual void addTrack(Track* track);
    virtual void addTrack(std::unique_ptr<Track> track);
    virtual Composer* getComposer() = 0;
    virtual std::vector<std::unique_ptr<Track>>::iterator findTrack(Track* track);
    virtual std::vector<std::unique_ptr<Track>>& getTracks();

private:
    std::vector<std::unique_ptr<Track>> _tracks;

};

