#include "Config.h"
#include "util.h"

Config gConfig;
Preference gPreference;
Theme gTheme;

Config::Config() {
    _dir = std::filesystem::path(GetExecutablePath()) / L"system" / L"config";
    std::filesystem::create_directories(_dir);
}

std::filesystem::path Config::projectDir() {
    auto dir = std::filesystem::path(GetExecutablePath()) / L"user" / L"project";
    std::filesystem::create_directories(dir);
    return dir;
}

