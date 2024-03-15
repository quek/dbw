#include "SerializeContext.h"

void SerializeContext::edit(nlohmann::json& json)
{
    json["__version"] = VERSION;
}

Neko* SerializeContext::findNeko(NekoId nekoId)
{
    return _nekoMap[nekoId];
}

void SerializeContext::store(Neko* neko)
{
    _nekoMap[neko->getNekoId()] = neko;
}
