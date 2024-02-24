#pragma once
#include <string>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "GuiUtil.h"
#include "Neko.h"


class Nameable : public Neko {
public:
    Nameable(const nlohmann::json& json);
    Nameable(const Nameable& other) = default;
    Nameable(const std::string& name = "");
    virtual ~Nameable() = default;
    virtual nlohmann::json toJson() override;

    std::string _name;
    ImVec4 _color = ImVec4(0.5, 0.5, 0.5, 0.5);


    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Nameable, _name, _color);
};
