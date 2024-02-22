#pragma once
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <tinyxml2/tinyxml2.h>


class Neko {
public:
    Neko();
    Neko(const nlohmann::json& json);
    Neko(const Neko& other);
    virtual ~Neko();
    virtual tinyxml2::XMLElement* toXml(tinyxml2::XMLDocument* /*doc*/) { return nullptr; }
    virtual const uint64_t nekoId() const;
    virtual void setNekoId(uint64_t id);
    virtual nlohmann::json toJson();

    static std::map<uint64_t, Neko*> nekoIdMap;
    template<typename T>
    static T* findByNekoId(uint64_t id) {
        auto it = nekoIdMap.find(id);
        if (it != nekoIdMap.end()) {
            return dynamic_cast<T*>(it->second);
        }
        return nullptr;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Neko, _nekoId);

private:
    void setNewNekoId();

    uint64_t _nekoId;
};

nlohmann::json eraseNekoId(const nlohmann::json& josn);
