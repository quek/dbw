#include "Neko.h"

std::map<NekoId, Neko*> Neko::nekoIdMap;

NekoId generateNeko() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    NekoId neko = static_cast<NekoId>(nanoseconds);
    while (true) {
        if (!Neko::nekoIdMap.contains(neko)) {
            return neko;
        }
        ++neko;
    }
}

Neko::Neko() {
    setNewNekoId();
}

Neko::Neko(const nlohmann::json& json) {
    if (json.contains("_nekoId") && json["_nekoId"].is_number()) {
        _nekoId = json["_nekoId"];
        nekoIdMap[_nekoId] = this;
    } else {
        setNewNekoId();
    }
}

Neko::Neko(const Neko&) {
    setNewNekoId();
}

Neko::~Neko() {
    nekoIdMap.erase(_nekoId);
}

const NekoId Neko::getNekoId() const {
    return _nekoId;
}

void Neko::setNekoId(NekoId id) {
    nekoIdMap.erase(_nekoId);
    _nekoId = id;
    nekoIdMap[_nekoId] = this;
}

nlohmann::json Neko::toJson() {
    return *this;
}

void Neko::setNewNekoId() {
    _nekoId = generateNeko();
    nekoIdMap[_nekoId] = this;
}

nlohmann::json eraseNekoId(const nlohmann::json& json) {
    if (json.is_array()) {
        nlohmann::json array = nlohmann::json::array();
        for (const auto& x : json) {
            array.push_back(eraseNekoId(x));
        }
        return array;
    }
    if (json.is_object()) {
        nlohmann::json object;
        for (auto& x : json.items()) {
            if (x.key() == "_nekoId") {
                continue;
            }
            object[x.key()] = eraseNekoId(x.value());
        }
        return object;
    }
    return json;
}

nlohmann::json renewNekoId(const nlohmann::json& json, std::map<NekoId, NekoId>& nekoIdMap) {
    if (json.is_array()) {
        nlohmann::json array = nlohmann::json::array();
        for (const auto& x : json) {
            array.push_back(renewNekoId(x, nekoIdMap));
        }
        return array;
    }
    if (json.is_object()) {
        nlohmann::json object;
        for (auto& x : json.items()) {
            if (x.key() == "_nekoId") {
                NekoId newNekoId = generateNeko();
                NekoId oldNekoId = x.value();
                nekoIdMap[oldNekoId] = newNekoId;
                object["_nekoId"] = newNekoId;
            } else {
                object[x.key()] = renewNekoId(x.value(), nekoIdMap);
            }
        }
        return object;
    }
    return json;
}

nlohmann::json renewNekoRef(const nlohmann::json& json, std::map<NekoId, NekoId>& nekoIdMap) {
    if (json.is_array()) {
        nlohmann::json array = nlohmann::json::array();
        for (const auto& x : json) {
            array.push_back(renewNekoRef(x, nekoIdMap));
        }
        return array;
    }
    if (json.is_object()) {
        nlohmann::json object;
        for (auto& x : json.items()) {
            if (x.key().ends_with("NekoRef")) {
                object[x.key()] = nekoIdMap[x.value()];
            } else {
                object[x.key()] = renewNekoRef(x.value(), nekoIdMap);
            }
        }
        return object;
    }
    return json;
}

nlohmann::json renewNekoId(const nlohmann::json& json) {
    std::map<NekoId, NekoId> nekoIdMap;
    auto renewNekoIdJson = renewNekoId(json, nekoIdMap);
    auto renewNekoRefJson = renewNekoRef(renewNekoIdJson, nekoIdMap);
    return renewNekoRefJson;
    
}
