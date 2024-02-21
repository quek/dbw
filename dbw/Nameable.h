#pragma once
#include <string>
#include "Neko.h"

class Nameable : public Neko {
public:
    Nameable(const Nameable& other) = default;
    Nameable(std::string name = "");
    virtual ~Nameable() = default;
    virtual nlohmann::json toJson() override;

    std::string _name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Nameable, _name);
};
