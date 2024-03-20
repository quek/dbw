#include "EditedParamIdListUpdate.h"

command::EditedParamIdListUpdate::EditedParamIdListUpdate(Param* param) : Command(false), _param(param)
{
}

void command::EditedParamIdListUpdate::execute(Composer*)
{
    _param->moduleGet()->updateEditedParamIdList(_param->getId());
}

void command::EditedParamIdListUpdate::undo(Composer*)
{
}
