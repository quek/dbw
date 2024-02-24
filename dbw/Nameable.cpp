#include "Nameable.h"

Nameable::Nameable(const nlohmann::json& json) : Neko(json) {
    _name = json["_name"];
    _color = json["_color"];
}

Nameable::Nameable(const std::string& name) : _name(name) {
}

nlohmann::json Nameable::toJson() {
    nlohmann::json json = Neko::toJson();
    json.update(*this);
    return json;
}

