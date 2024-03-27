#pragma once
#include "../Command.h"

namespace command {
class ModuleDisconnect : public Command
{
public:
    ModuleDisconnect(Connection* connection);
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

