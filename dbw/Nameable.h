#pragma once
#include <string>

class Nameable {
public:
    Nameable(const Nameable& other) = default;
    Nameable(std::string name = "");
    virtual ~Nameable() = default;

    std::string _name;
};
