#pragma once
#include <json.hpp>

struct Preference {
    void load();
    void save();

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

struct Theme {

};

class Config {
public:
    Config();

    std::filesystem::path _dir;
    Theme _theme;
};

extern Config gConfig;
extern Preference gPreference;
