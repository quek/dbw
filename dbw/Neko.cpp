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

const NekoId Neko::nekoId() const {
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
