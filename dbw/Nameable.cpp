#include "Nameable.h"

Nameable::Nameable(std::string name) : _name(name) {
}

nlohmann::json Nameable::toJson() {
    nlohmann::json json = Neko::toJson();
    json.update(*this);
    return json;
}

