#include "PluginManager.h"
#include <fstream>
#include <filesystem>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "windows.h"
#include "Composer.h"
#include "GainModule.h"
#include "logger.h"
#include "PluginHost.h"
#include "PluginModule.h"
#include "Track.h"
#include "util.h"
#include "Vst3Module.h"

std::map<std::string, std::function<BuiltinModule* (Track*)>> builtinModuleMap = {
    {"Gain", [](Track* track) -> BuiltinModule* { return new GainModule("Gain", track); }},
};

void CreateConfigDirectoryAndSaveFile(const std::string& directory, const std::string& filename, std::string content) {
    std::string configPath = directory + "\\config";
    std::string fullPath = configPath + "\\" + filename;
    CreateDirectoryA(configPath.c_str(), NULL);

    std::ofstream configFile(fullPath);
    configFile << content << std::endl;
    configFile.close();
}

PluginManager::PluginManager(Composer* composer) : _composer(composer) {
}

Module* PluginManager::create(tinyxml2::XMLElement* element, Track* track) {
    if (strcmp(element->Name(), "ClapPlugin") == 0) {
        auto deviceId = element->Attribute("deviceID");
        auto plugin = _composer->_pluginManager.findPlugin("clap", deviceId);
        if (plugin == nullptr) {
            return nullptr;
        }
        PluginHost* pluginHost = new PluginHost(track);
        pluginHost->load((*plugin)["path"].get<std::string>(), (*plugin)["index"].get<uint32_t>());
        Module* module = new PluginModule(pluginHost->_name, track, pluginHost);
        auto state = element->FirstChildElement("State");
        pluginHost->_statePath = state->Attribute("path");
        pluginHost->loadState();
        return module;
    } else if (strcmp(element->Name(), "Vst3Plugin") == 0) {
        auto deviceId = element->Attribute("deviceID");
        auto plugin = _composer->_pluginManager.findPlugin("vst3", deviceId);
        if (plugin == nullptr) {
            return nullptr;
        }
        auto path = (*plugin)["path"].get<std::string>();
        auto module = new Vst3Module((*plugin)["name"], track);
        module->load(path);
        auto state = element->FirstChildElement("State");
        auto statePath = _composer->_project->projectDir() / state->Attribute("path");
        module->loadState(statePath);
        return module;
    } else if (strcmp(element->Name(), "BuiltinDevice") == 0) {
        auto deviceId = element->Attribute("deviceID");
        auto factory = builtinModuleMap.find(deviceId);
        if (factory == builtinModuleMap.end()) {
            return nullptr;
        }
        BuiltinModule* module = (*factory).second(track);
        module->loadParameters(element->FirstChildElement("Parameters"));
        return module;
    }
    return nullptr;
}

void PluginManager::scan() {
    scanClap();
    scanVst3();
}

void PluginManager::load() {
    auto path = configDir() / "plugin.json";
    std::ifstream in(path);
    if (in.is_open()) {
        in >> _plugins;
    } else {
        scan();
    }
}

void PluginManager::openModuleSelector(Track* track) {
    ImGui::Begin("Module Selector", &track->_openModuleSelector);
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
    }
    ImGui::InputText("##query", &_query, ImGuiInputTextFlags_AutoSelectAll);

    for (auto plugin = _plugins["clap"].begin(); plugin != _plugins["clap"].end(); ++plugin) {
        auto q = _query.begin();
        std::string name = (*plugin)["name"].get<std::string>();
        for (auto c = name.begin(); c != name.end() && q != _query.end(); ++c) {
            if (std::tolower(*q) == std::tolower(*c)) {
                ++q;
            }
        }
        if (q == _query.end()) {
            if (ImGui::Button(name.c_str())) {
                track->_openModuleSelector = false;
                track->addModule((*plugin)["path"].get<std::string>(), (*plugin)["index"].get<uint32_t>());
            }
        }
    }

    for (auto& plugin : _plugins["vst3"]) {
        auto q = _query.begin();
        std::string name = plugin["name"].get<std::string>();
        for (auto c = name.begin(); c != name.end() && q != _query.end(); ++c) {
            if (std::tolower(*q) == std::tolower(*c)) {
                ++q;
            }
        }
        if (q == _query.end()) {
            if (ImGui::Button(name.c_str())) {
                track->_openModuleSelector = false;
                auto path = plugin["path"].get<std::string>();
                auto module = new Vst3Module(name, track);
                module->load(path);
                module->openGui();
                // TODO UNDO
                track->_modules.emplace_back(module);
            }
        }
    }

    for (const auto& [name, fun] : builtinModuleMap) {
        auto q = _query.begin();
        for (auto c = name.begin(); c != name.end() && q != _query.end(); ++c) {
            if (std::tolower(*q) == std::tolower(*c)) {
                ++q;
            }
        }
        if (q == _query.end()) {
            if (ImGui::Button(name.c_str())) {
                track->_openModuleSelector = false;
                auto module = fun(track);
                _composer->_commandManager.executeCommand(new AddModuleCommand(track, module));
            }
        }
    }
    //ImGuiStyle& style = ImGui::GetStyle();
    //int buttons_count = 20;
    //float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    //for (int n = 0; n < buttons_count; n++)
    //{
    //    ImGui::PushID(n);
    //    ImGui::Button("Box", button_sz);
    //    float last_button_x2 = ImGui::GetItemRectMax().x;
    //    float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
    //    if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
    //        ImGui::SameLine();
    //    ImGui::PopID();
    //}
    ImGui::End();
}

nlohmann::json* PluginManager::findPlugin(const char* pluginType, const std::string deviceId) {
    auto plugin = std::find_if(_plugins[pluginType].begin(), _plugins[pluginType].end(), [&deviceId](auto x) { return x["id"] == deviceId; });
    if (plugin != _plugins[pluginType].end()) {
        return &*plugin;
    }
    return nullptr;
}

void PluginManager::scanClap() {
    std::string clapPluginDir = "C:\\Program Files\\Common Files\\CLAP";
    std::vector<std::string> pluginPaths;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(clapPluginDir)) {
            if (!entry.is_directory() && entry.path().extension() == ".clap") {
                pluginPaths.push_back(entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // ディレクトリの探索中にエラーが発生した場合のエラー処理
        logger->error("Filesystem error: {}", e.what());
    }

    _plugins["clap"] = nlohmann::json::array();
    for (auto i = pluginPaths.begin(); i != pluginPaths.end(); ++i) {
        auto x = PluginHost::scan(*i);
        for (auto p = x.begin(); p != x.end(); ++p) {
            _plugins["clap"].push_back(*p);
        }
    }

    auto path = configDir() / "plugin.json";
    std::ofstream configFile(path);
    configFile << _plugins.dump(2) << std::endl;
    configFile.close();
}

void PluginManager::scanVst3() {
    std::string pluginDir = "C:\\Program Files\\Common Files\\VST3";
    std::vector<std::string> pluginPaths;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(pluginDir)) {
            if (!entry.is_directory() && entry.path().extension() == ".vst3") {
                pluginPaths.push_back(entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // ディレクトリの探索中にエラーが発生した場合のエラー処理
        logger->error("Filesystem error: {}", e.what());
    }

    _plugins["vst3"] = nlohmann::json::array();
    for (auto i = pluginPaths.begin(); i != pluginPaths.end(); ++i) {
        auto x = Vst3Module::scan(*i);
        _plugins["vst3"].push_back(x);
    }

    auto path = configDir() / "plugin.json";
    std::ofstream configFile(path);
    configFile << _plugins.dump(2) << std::endl;
    configFile.close();
}
