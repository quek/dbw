#pragma once
#include "Neko.h"

class SerializeContext
{
public:
    inline static const int VERSION = 1;
    void edit(nlohmann::json& json);
    Neko* findNeko(NekoId nekoId);
    void store(Neko* neko);

private:
    std::map<NekoId, Neko*> _nekoMap;
};

