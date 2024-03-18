#include "Config.h"
#include "util.h"

Config gConfig;
Preference gPreference;
Theme gTheme;

Config::Config()
{
    _dir = std::filesystem::path(GetExecutablePath()) / L"user" / L"config";
    std::filesystem::create_directories(_dir);
}

std::filesystem::path Config::projectDir()
{
    auto dir = std::filesystem::path(GetExecutablePath()) / L"user" / L"project";
    std::filesystem::create_directories(dir);
    return dir;
}

nlohmann::json Preference::to_json()
{
    return nlohmann::json{
        {"audioDeviceIndex", audioDeviceIndex},
        {"sampleRate", sampleRate },
        { "bufferSize", bufferSize },
        { "midiInDevices", midiInDevices },
    };
}

void Preference::from_json(const nlohmann::json& json)
{
    for (const auto& x : json.items())
    {
        if (x.key() == "audioDeviceIndex")
        {
            audioDeviceIndex = x.value();
        }
        else if (x.key() == "sampleRate")
        {
            sampleRate = x.value();
        }
        else if (x.key() == "bufferSize")
        {
            bufferSize = x.value();
        }
        else if (x.key() == "midiInDevices")
        {
            midiInDevices = x.value();
        }
    }
}

void Theme::from_json(const nlohmann::json& json)
{
    for (const auto& x : json.items())
    {
        if (x.key() == "automationLine")
        {
            automationLine = x.value();
        }
        else if (x.key() == "automationPoint")
        {
            automationPoint = x.value();
        }
        else if (x.key() == "sequenceEnd")
        {
            sequenceEnd = x.value();
        }
    }
}

nlohmann::json Theme::to_json()
{
    return nlohmann::json{
        {"automationLine", automationLine },
        {"automationPoint", automationPoint},
        {"sequenceEnd", sequenceEnd },
    };
}
