#include "Config.h"
#include <fstream>
#include "util.h"

Config gConfig;
Preference gPreference;

Config::Config() {
    _dir = std::filesystem::path(GetExecutablePath()) / "system" / "config";
    std::filesystem::create_directories(_dir);
}

void Preference::load() {
    std::ifstream in(gConfig._dir / "preference.json");
    if (!in) {
        return;
    }
    nlohmann::json json;
    in >> json;
    gPreference = json.template get<Preference>();
}

void Preference::save() {
    std::ofstream out(gConfig._dir / "preference.json");
    nlohmann::json json = gPreference;
    out << json.dump(2) << std::endl;
}
