#pragma once
#include <string>
#include "../Command.h"

class Module;
class Track;

namespace command {

class AddModule : public Command {
public:
    AddModule(NekoId trackRef, const char* type, const std::string& id, bool openGui=true);
    void execute(Composer* composer) override;
    void redo(Composer* composer) override;
    void undo(Composer* composer) override;
    NekoId _trackRef;
    NekoId _moduleId = 0;
    std::string _type;
    std::string _id;
private:
    Module* exec(Composer* composer);
    bool _openGui;
};


};
