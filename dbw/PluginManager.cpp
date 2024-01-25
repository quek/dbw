#include "PluginManager.h"
#include "util.h"
#include "Windows.h":
#include <fstream>
#include <filesystem>
#include "logging.h"
#include "PluginHost.h"

void CreateConfigDirectoryAndSaveFile(const std::string& directory, const std::string& filename) {
    std::string configPath = directory + "\\config";
    std::string fullPath = configPath + "\\" + filename;
    CreateDirectoryA(configPath.c_str(), NULL);

    std::ofstream configFile(fullPath);
    configFile << "設定データ" << std::endl;
    configFile << "プラグイン";
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
    }
    catch (const std::filesystem::filesystem_error& e) {
        // ディレクトリの探索中にエラーが発生した場合のエラー処理
        logger->error("Filesystem error: {}", e.what());
    }

    for (auto i = pluginPaths.begin(); i != pluginPaths.end(); ++i) {
        PluginHost pluginHost;
        pluginHost.scan(*i);
    }

    std::string exePath = GetExecutablePath();
    CreateConfigDirectoryAndSaveFile(exePath, "config.txt");
}

/*
  <Plug>
    <PluginFormat>CLAP</PluginFormat>
    <FilePath>/C:/Program Files/Common Files/CLAP/Surge Synth Team/Surge XT.clap</FilePath>
    <Name/>
    <PluginSdkVersion>1.1.8</PluginSdkVersion>
    <FileSize>20382208</FileSize>
    <ExeBitSize>64</ExeBitSize>
    <Enabled>1</Enabled>
    <Favorite>0</Favorite>
    <LastDataFilePath/>
    <PrefNumAuxAudioInputs>0</PrefNumAuxAudioInputs>
    <PrefNumAuxAudioOutputs>0</PrefNumAuxAudioOutputs>
    <PrefNumAuxEventInputs>0</PrefNumAuxEventInputs>
    <PrefNumAuxEventOutputs>0</PrefNumAuxEventOutputs>
    <Categories/>
    <MustZeroOutputsBeforeProcessReplacing>0</MustZeroOutputsBeforeProcessReplacing>
    <ClapVersion>1.1.8</ClapVersion>
    <Id>org.surge-synth-team.surge-xt</Id>
    <Name/>
    <Vendor>Surge Synth Team</Vendor>
    <Url>https://surge-synth-team.org/</Url>
    <ManualUrl/>
    <SupportUrl/>
    <Version>1.3.0</Version>
    <Description>Surge XT</Description>
    <Features>
      <Tag>instrument</Tag>
      <Tag>synthesizer</Tag>
      <Tag>hybrid</Tag>
      <Tag>free and open source</Tag>
    </Features>
  </Plug>
*/
