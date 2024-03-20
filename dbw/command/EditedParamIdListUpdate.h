#pragma once
#include "../Command.h"

class Param;
namespace command {
class EditedParamIdListUpdate : public Command
{
public:
    EditedParamIdListUpdate(Param* param);
    void execute(Composer* composer) override;
    void undo(Composer* composer) override;
private:
    Param* _param;
};
};

