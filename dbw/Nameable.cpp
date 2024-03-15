#include "Nameable.h"

Nameable::Nameable(const nlohmann::json& json, SerializeContext& context) : Neko(json, context) {
    _name = json["_name"];
    _color = json["_color"];
}

Nameable::Nameable(const std::string& name) : _name(name) {
}

nlohmann::json Nameable::toJson(SerializeContext& context) {
    nlohmann::json json = Neko::toJson(context);
    json.update(*this);
    return json;
}

