#include "PluginManager.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "util.h"
#include "Windows.h"
#include <fstream>
#include <filesystem>
#include "logging.h"
#include "PluginHost.h"
#include "Composer.h"
#include "Track.h"


void CreateConfigDirectoryAndSaveFile(const std::string& directory, const std::string& filename, std::string content) {
    std::string configPath = directory + "\\config";
    std::string fullPath = configPath + "\\" + filename;
    CreateDirectoryA(configPath.c_str(), NULL);

    std::ofstream configFile(fullPath);
    configFile << content << std::endl;
    configFile.close();
}

void PluginManager::scan()
{
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

    _plugins.clear();
    for (auto i = pluginPaths.begin(); i != pluginPaths.end(); ++i) {
        PluginHost pluginHost;
        auto x = pluginHost.scan(*i);
        for (auto p = x.begin(); p != x.end(); ++p) {
            _plugins["clap"].push_back(*p);
        }
    }

    std::string exePath = GetExecutablePath();
    CreateConfigDirectoryAndSaveFile(exePath, "plugin.json", _plugins.dump(2));
}

void PluginManager::load()
{
    std::string configFile = GetExecutablePath() + "\\config\\plugin.json";
    std::ifstream in(configFile);
    if (in.is_open()) {
        in >> _plugins;
    } else {
        scan();
    }
}

void PluginManager::openModuleSelector(Track* track)
{
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

