#pragma once
#include <string>
#include "../Command.h"

class Module;
class Track;

namespace command {

class AddModule : public Command {
public:
    AddModule(uint64_t trackRef, const char* type, const std::string& id);
    void execute(Composer* composer) override;
    void redo(Composer* composer) override;
    void undo(Composer* composer) override;
    uint64_t _trackRef;
    uint64_t _moduleId = 0;
    std::string _type;
    std::string _id;
private:
    Module* exec(Composer* composer);
};


};
