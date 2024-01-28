#pragma once

#include <filesystem>
#include <string>
#include <clap/clap.h>
#include <windows.h>
#include <json.hpp>
#include "PluginEventList.h"

class ProcessBuffer;
class Track;

class PluginHost {
public:
    PluginHost(Track* track);
    ~PluginHost();
    bool load(const std::string path, uint32_t pluginIndex);
    static nlohmann::json scan(const std::string path);
    void unload();
    clap_process* process(ProcessBuffer* in, uint32_t bufferSize, int64_t steadyTime);
    void openGui();
    void closeGui();
    bool canUseGui() const noexcept;
    void start(double sampleRate, uint32_t bufferSize);
    void stop();
    void loadState();
    void saveState();

    std::string _name;
    const clap_plugin* _plugin = nullptr;
    double _sampleRate = 0;
    uint32_t _bufferSize = 0;
    std::filesystem::path _statePath;

private:
    HMODULE _library = nullptr;
    bool _gui = false;
    clap_window _clap_window = {};
    HWND _hwnd = {};
    WNDCLASSEXW _wndClass = {};
    clap_host _clap_host;
    const clap_plugin_gui* _pluginGui = nullptr;
    const clap_plugin_audio_ports* _pluginAudioPorts = nullptr;
    const clap_plugin_state* _pluginState = nullptr;
    bool _processing = false;

    /* process stuff */
    unsigned long _allocatedSize = 0;
    float* _inputs[2] = { nullptr, nullptr };
    float* _outputs[2] = { nullptr, nullptr };
    PluginEventList _evIn;
    PluginEventList _evOut;
    clap_process _process = {};
    clap_audio_buffer _audioIn = {};
    clap_audio_buffer _audioOut = {};

    Track* _track;


    static const void* clapGetExtension(const clap_host_t* host, const char* extension_id) noexcept;
    static void clapRequestRestart(const clap_host_t* host) noexcept;
    static void clapRequestProcess(const clap_host_t* host) noexcept;
    static void clapRequestCallback(const clap_host_t* host) noexcept;

    static const clap_host_audio_ports _hostAudioPorts;
};

