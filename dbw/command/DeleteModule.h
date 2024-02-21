#pragma once
#include "../Command.h"

class Composer;
class Module;

namespace command {

class DeleteModule: public Command {
public:
    DeleteModule(Module* module);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    uint64_t _moduleId;
    uint64_t _trackId;
    nlohmann::json _module;
    size_t _index = 0;
};

};
