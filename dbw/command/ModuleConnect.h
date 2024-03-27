#pragma once
#include "../Command.h"

namespace command
{
class ModuleConnect : public Command
{
public:
    ModuleConnect(Module* from, int fromIndex, Module* to, int toIndex, bool post = true);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    NekoId _fromNekoRef;
    int _fromIndex;
    NekoId _toNekoRef;
    int _toIndex;
    bool _post;
};
};

