#include "Neko.h"

std::map<uint64_t, Neko*> Neko::nekoIdMap;

uint64_t generateNeko() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    uint64_t neko = static_cast<uint64_t>(nanoseconds);
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

const uint64_t Neko::nekoId() const {
    return _nekoId;
}

void Neko::setNekoId(uint64_t id) {
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
