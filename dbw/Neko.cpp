#include "Neko.h"
#include "SerializeContext.h"

std::map<NekoId, Neko*> Neko::idNekoMap;

NekoId generateNeko()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    NekoId neko = static_cast<NekoId>(nanoseconds);
    while (true)
    {
        if (!Neko::idNekoMap.contains(neko))
        {
            return neko;
        }
        ++neko;
    }
}

Neko::Neko()
{
    setNewNekoId();
}

Neko::Neko(const nlohmann::json& json, SerializeContext&)
{
    if (json.contains("_nekoId") && json["_nekoId"].is_number())
    {
        _nekoId = json["_nekoId"];
        idNekoMap[_nekoId] = this;
    }
    else
    {
        setNewNekoId();
    }
}

Neko::Neko(const Neko&)
{
    setNewNekoId();
}

Neko::~Neko()
{
    idNekoMap.erase(_nekoId);
}

const NekoId Neko::getNekoId() const
{
    return _nekoId;
}

void Neko::setNekoId(NekoId id)
{
    idNekoMap.erase(_nekoId);
    _nekoId = id;
    idNekoMap[_nekoId] = this;
}

nlohmann::json Neko::toJson(SerializeContext& context)
{
    context.store(this);
    return *this;
}

void Neko::setNewNekoId()
{
    _nekoId = generateNeko();
    idNekoMap[_nekoId] = this;
}

nlohmann::json eraseNekoId(const nlohmann::json& json)
{
    if (json.is_array())
    {
        nlohmann::json array = nlohmann::json::array();
        for (const auto& x : json)
        {
            array.push_back(eraseNekoId(x));
        }
        return array;
    }
    if (json.is_object())
    {
        nlohmann::json object;
        for (auto& x : json.items())
        {
            if (x.key() == "_nekoId")
            {
                continue;
            }
            object[x.key()] = eraseNekoId(x.value());
        }
        return object;
    }
    return json;
}

nlohmann::json renewNekoId(const nlohmann::json& json, std::map<NekoId, NekoId>& renewIdMap)
{
    if (json.is_array())
    {
        nlohmann::json array = nlohmann::json::array();
        for (const auto& x : json)
        {
            array.push_back(renewNekoId(x, renewIdMap));
        }
        return array;
    }
    if (json.is_object())
    {
        nlohmann::json object;
        for (auto& x : json.items())
        {
            if (x.key() == "_nekoId")
            {
                NekoId newNekoId = generateNeko();
                NekoId oldNekoId = x.value();
                renewIdMap[oldNekoId] = newNekoId;
                object["_nekoId"] = newNekoId;
            }
            else
            {
                object[x.key()] = renewNekoId(x.value(), renewIdMap);
            }
        }
        return object;
    }
    return json;
}

nlohmann::json renewNekoRef(const nlohmann::json& json, std::map<NekoId, NekoId>& renewIdMap)
{
    if (json.is_array())
    {
        nlohmann::json array = nlohmann::json::array();
        for (const auto& x : json)
        {
            array.push_back(renewNekoRef(x, renewIdMap));
        }
        return array;
    }
    if (json.is_object())
    {
        nlohmann::json object;
        for (auto& x : json.items())
        {
            if (x.key().ends_with("NekoRef"))
            {
                NekoId nekoRef = x.value();
                if (renewIdMap.contains(nekoRef))
                {
                    object[x.key()] = renewIdMap[nekoRef];
                }
                else
                {
                    object[x.key()] = x.value();
                }
            }
            else
            {
                object[x.key()] = renewNekoRef(x.value(), renewIdMap);
            }
        }
        return object;
    }
    return json;
}

nlohmann::json renewNekoId(const nlohmann::json& json)
{
    std::map<NekoId, NekoId> renewIdMap;
    auto renewNekoIdJson = renewNekoId(json, renewIdMap);
    auto renewNekoRefJson = renewNekoRef(renewNekoIdJson, renewIdMap);
    return renewNekoRefJson;

}
