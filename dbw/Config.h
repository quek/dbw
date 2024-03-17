#pragma once
#include <fstream>
#include <string>
#include <vector>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <nlohmann/json.hpp>
#include "GuiUtil.h"

class Config;
struct Preference;
struct Theme;

extern Config gConfig;
extern Preference gPreference;
extern Theme gTheme;

class Config
{
public:
    Config();
    std::filesystem::path projectDir();

    std::filesystem::path _dir;
};

struct ConfigMixin
{
    explicit ConfigMixin(std::filesystem::path _fileName) : fileName(_fileName) {}

    virtual void from_json(const nlohmann::json& json) = 0;
    virtual nlohmann::json to_json() = 0;
    void load()
    {
        std::ifstream in(gConfig._dir / fileName);
        if (!in)
        {
            return;
        }
        nlohmann::json json;
        in >> json;
        from_json(json);
    }

    void save()
    {
        std::ofstream out(gConfig._dir / fileName);
        nlohmann::json json= to_json();
        out << json.dump(2) << std::endl;
    }

    std::filesystem::path fileName;
};

struct Preference : ConfigMixin
{
    Preference() : ConfigMixin(L"preference.json") {}
    void from_json(const nlohmann::json& json) override;
    nlohmann::json to_json() override;
    

    int audioDeviceIndex = -1;
    double sampleRate = 48000.0;
    unsigned long bufferSize = 1024;
    std::vector<std::string> midiInDevices;
};

struct Theme : ConfigMixin
{
    Theme() : ConfigMixin(L"theme.json") {}
    void from_json(const nlohmann::json& json) override;
    nlohmann::json to_json() override;

    ImU32 automationLine = IM_COL32(0x80, 0x80, 0xff, 0xee);
    ImU32 automationPoint = IM_COL32(0xcc, 0xcc, 0xff, 0xcc);

    ImU32 editCursor = IM_COL32(0x00, 0xff, 0x00, 0x80);

    ImU32 rackBorder = IM_COL32(0x80, 0x80, 0x80, 0x80);

    ImU32 buttonOn = IM_COL32(0x40, 0x80, 0x80, 0x90);
    ImU32 buttonOnHovered = IM_COL32(0x60, 0xa0, 0xa0, 0x90);
    ImU32 buttonOnActive = IM_COL32(0x50, 0x90, 0x90, 0x80);

    ImU32 background = IM_COL32(0x00, 0x00, 0x00, 0x80);
    ImU32 text = IM_COL32(0xff, 0xff, 0xff, 0xc0);
};

