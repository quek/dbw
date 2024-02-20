#pragma once
#include <string>
#include "../Command.h"

class Module;
class Track;

namespace command {

class AddModule2 : public Command {
public:
    AddModule2(uint64_t trackRef, std::string& type, std::string& id);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    uint64_t _trackRef;
    std::string _type;
    std::string _id;
};

class AddModule : public Command {
public:
    AddModule(Track* track, Module* module) : _track(track), _module(module) {}
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
    Track* _track;
    std::unique_ptr<Module> _module;
};


};
