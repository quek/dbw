#pragma once
#include <fstream>
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

class Config {
public:
    Config();
    std::filesystem::path projectDir();

    std::filesystem::path _dir;
};

template<typename T>
struct ConfigMixin {
    explicit ConfigMixin(const char* _fileName) : fileName(_fileName) {}

    void load() {
        std::ifstream in(gConfig._dir / fileName);
        if (!in) {
            return;
        }
        nlohmann::json json;
        in >> json;
        *static_cast<T*>(this) = json.template get<T>();
    }

    void save() {
        std::ofstream out(gConfig._dir / fileName);
        nlohmann::json json = *static_cast<T*>(this);
        out << json.dump(2) << std::endl;
    }

    const char* fileName;
};

struct Preference : ConfigMixin<Preference> {
    Preference() : ConfigMixin("preference.json") {}

    int audioDeviceIndex = -1;
    double sampleRate = 48000.0;
    unsigned long bufferSize = 1024;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        Preference,
        audioDeviceIndex,
        sampleRate,
        bufferSize
    );
};

struct Theme : ConfigMixin<Theme>{
    Theme() : ConfigMixin("theme.json") {}

    ImU32 automationLine = IM_COL32(0xcc, 0xcc, 0xff, 0xcc);
    ImU32 automationPoint = IM_COL32(0x80, 0x80, 0xff, 0xee);

    ImU32 editCursor = IM_COL32(0x00, 0xff, 0x00, 0x80);

    ImU32 rackBorder = IM_COL32(0x80, 0x80, 0x80, 0x80);

    ImU32 background = IM_COL32(0x00, 0x00, 0x00, 0x80);
    ImU32 text = IM_COL32(0xff, 0xff, 0xff, 0xc0);

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        Theme,
        automationPoint,
        editCursor,
        rackBorder,
        background
    );
};

