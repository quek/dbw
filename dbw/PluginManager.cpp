#include "PluginManager.h"
#include "imgui.h"
#include "util.h"
#include "Windows.h"
#include <fstream>
#include <filesystem>
#include "logging.h"
#include "PluginHost.h"
#include "Composer.h"


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
    ImGui::Text("nya~");
    ImGui::End();
}

