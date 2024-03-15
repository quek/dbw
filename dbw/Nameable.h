#pragma once
#include <string>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "GuiUtil.h"
#include "Neko.h"


class Nameable : public Neko {
public:
    Nameable(const nlohmann::json& json, SerializeContext& context);
    Nameable(const Nameable& other) = default;
    Nameable(const std::string& name = "");
    virtual ~Nameable() = default;
    virtual nlohmann::json toJson(SerializeContext& context) override;

    std::string _name;
    ImU32 _color = IM_COL32(0x80, 0x80, 0x80, 0x80);


    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Nameable, _name, _color);
};
