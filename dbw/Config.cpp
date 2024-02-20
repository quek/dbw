#include "Config.h"
#include "util.h"

Config gConfig;
Preference gPreference;
Theme gTheme;

Config::Config() {
    _dir = std::filesystem::path(GetExecutablePath()) / "system" / "config";
    std::filesystem::create_directories(_dir);
}

