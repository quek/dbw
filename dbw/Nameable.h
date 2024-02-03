#pragma once
#include <string>

class Nameable {
public:
    Nameable(std::string name = "");
    virtual ~Nameable() = default;

    std::string _name;
};
