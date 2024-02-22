#include "ClapHost.h"
#include <fstream>
#include "Composer.h"
#include "Error.h"
#include "logger.h"
#include "util.h"

LRESULT WINAPI PluginHostWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const clap_host_audio_ports ClapHost::_hostAudioPorts = {
    // Checks if the host allows a plugin to change a given aspect of the audio ports definition.
    // [main-thread]
    // bool(CLAP_ABI *is_rescan_flag_supported)(const clap_host_t *host, uint32_t flag);
     .is_rescan_flag_supported = [](const clap_host_t* /* host */, uint32_t /*flag*/) {
        return false;
    },
    // Rescan the full list of audio ports according to the flags.
    // It is illegal to ask the host to rescan with a flag that is not supported.
    // Certain flags require the plugin to be de-activated.
    // [main-thread]
    // void(CLAP_ABI * rescan)(const clap_host_t * host, uint32_t flags);
   .rescan = [](const clap_host_t* /*host*/, uint32_t /*flags*/) {}
};

const clap_host_latency ClapHost::_hostLatency = {
    .changed = [](const clap_host_t* host) {
        ClapHost* self = (ClapHost*)host->host_data;
        self->changeLatency();
    }
};

ClapHost::ClapHost(Track* track) : _track(track) {
    _clap_host = clap_host{
        // clap_version_t clap_version; // initialized to CLAP_VERSION
        .clap_version = CLAP_VERSION,
        // void *host_data; // reserved pointer for the host
        .host_data = this,
        // name and version are mandatory.
        // const char *name;    // eg: "Bitwig Studio"
        .name = "dbw",
        // const char *vendor;  // eg: "Bitwig GmbH"
        .vendor = "dbw",
        // const char *url;     // eg: "https://bitwig.com"
        .url = "https://github.com/quek/dbw",
        // const char *version; // eg: "4.3", see plugin.h for advice on how to format the version
        .version = "0.0.1",
        // Query an extension.
        // The returned pointer is owned by the host.
        // It is forbidden to call it before plugin->init().
        // You can call it within plugin->init() call, and after.
        // [thread-safe]
        // const void *(CLAP_ABI *get_extension)(const struct clap_host *host, const char *extension_id);
        .get_extension = clapGetExtension,
        // Request the host to deactivate and then reactivate the plugin.
        // The operation may be delayed by the host.
        // [thread-safe]
        // void(CLAP_ABI *request_restart)(const struct clap_host *host);
        .request_restart = clapRequestRestart,
        // Request the host to activate and start processing the plugin.
        // This is useful if you have external IO and need to wake up the plugin from "sleep".
        // [thread-safe]
        // void(CLAP_ABI *request_process)(const struct clap_host *host);
        .request_process = clapRequestProcess,
        // Request the host to schedule a call to plugin->on_main_thread(plugin) on the main thread.
        // [thread-safe]
        // void(CLAP_ABI *request_callback)(const struct clap_host *host);
        .request_callback = clapRequestCallback,
    };

    // Set to -1 if not available, otherwise the value must be greater or equal to 0,
    // and must be increased by at least `frames_count` for the next call to process.
    // int64_t steady_time;
    _process.steady_time = 0;
    // time info at sample 0
    // If null, then this is a free running host, no transport events will be provided
    _process.transport = nullptr;
    _process.audio_inputs_count = 1;
    _process.audio_outputs_count = 1;

    _audioIn.data32 = _inputs;
    _audioIn.data64 = nullptr;
    _audioIn.channel_count = 2;
    _audioIn.latency = 0;
    _audioIn.constant_mask = 0;
    _process.audio_inputs = &_audioIn;
    _audioOut.data32 = _outputs;
    _audioOut.data64 = nullptr;
    _audioOut.channel_count = 2;
    _audioOut.latency = 0;
    _audioOut.constant_mask = 0;
    _process.audio_outputs = &_audioOut;

    _statePath = std::filesystem::path("plugins") / (generateUniqueId() + ".clap-preset");
}

ClapHost::~ClapHost() {
    unload();
}

bool ClapHost::load(const std::string path, uint32_t pluginIndex) {
    std::wstring wstr(path.begin(), path.end());
    LPCWSTR lpwstr = wstr.c_str();
    _library = LoadLibrary(lpwstr);
    if (!_library) {
        // TODO
        logger->error("Load error {}", path);
        return false;
    }
    struct clap_plugin_entry* entry = (struct clap_plugin_entry*)GetProcAddress(_library, "clap_entry");
    entry->init(path.c_str());
    auto factory =
        static_cast<const clap_plugin_factory*>(entry->get_factory(CLAP_PLUGIN_FACTORY_ID));
    auto count = factory->get_plugin_count(factory);
    if (pluginIndex > count) {
        // TODO
        logger->error("pluginIndex {} > count {} is false", pluginIndex, count);
        return false;
    }

    auto desc = factory->get_plugin_descriptor(factory, pluginIndex);
    if (!desc) {
        logger->error("no plugin descriptor");
        return false;
    }

    if (!clap_version_is_compatible(desc->clap_version)) {
        logger->error("Incompatible clap version");
        return false;
    }

    _plugin = factory->create_plugin(factory, &_clap_host, desc->id);
    if (!_plugin) {
        logger->error("could not create the plugin with id: {}", desc->id);
        return false;
    }

    if (!_plugin->init(_plugin)) {
        logger->error("could not init the plugin with id: {}", desc->id);
        return false;
    }

    _pluginGui = static_cast<const clap_plugin_gui*>(_plugin->get_extension(_plugin, CLAP_EXT_GUI));
    _pluginAudioPorts = static_cast<const clap_plugin_audio_ports*>(_plugin->get_extension(_plugin, CLAP_EXT_AUDIO_PORTS));
    {
        uint32_t index = 0;
        bool isInput = true;
        clap_audio_port_info info;
        bool result = _pluginAudioPorts->get(_plugin, index, isInput, &info);
        logger->info("audio ports result: {}", result);
    }
    _pluginState = static_cast<const clap_plugin_state*>(_plugin->get_extension(_plugin, CLAP_EXT_STATE));
    _pluginLatency = static_cast<const clap_plugin_latency*>(_plugin->get_extension(_plugin, CLAP_EXT_LATENCY));
    changeLatency();

    _name = desc->name;

    return true;
}

nlohmann::json ClapHost::scan(const std::string path) {
    nlohmann::json plugins;
    std::wstring wstr(path.begin(), path.end());
    LPCWSTR lpwstr = wstr.c_str();
    auto library = LoadLibrary(lpwstr);
    if (!library) {
        // TODO
        logger->error("Load error {}", path);
        return plugins;
    }
    struct clap_plugin_entry* entry = (struct clap_plugin_entry*)GetProcAddress(library, "clap_entry");
    entry->init(path.c_str());
    auto factory =
        static_cast<const clap_plugin_factory*>(entry->get_factory(CLAP_PLUGIN_FACTORY_ID));
    auto count = factory->get_plugin_count(factory);

    for (uint32_t i = 0; i < count; ++i) {
        const clap_plugin_descriptor* desc = factory->get_plugin_descriptor(factory, i);
        if (!desc) {
            logger->error("no plugin descriptor");
            return plugins;
        }

        nlohmann::json json;
        json["id"] = desc->id;
        json["name"] = desc->name;
        json["vendor"] = desc->vendor;
        json["url"] = desc->url;
        json["manual_url"] = desc->manual_url;
        json["support_url"] = desc->support_url;
        json["version"] = desc->version;
        json["description"] = desc->description;
        nlohmann::json features;
        for (int j = 0; desc->features[j] != nullptr; ++j) {
            features.push_back(desc->features[j]);
        }
        json["features"] = features;
        json["path"] = path;
        json["index"] = i;
        plugins.push_back(json);
    }
    return plugins;
}

void ClapHost::unload() {
    if (!_plugin) {
        if (_library != nullptr) {
            FreeLibrary(_library);
            _library = nullptr;
        }
        return;
    }
    stop();
    _plugin->destroy(_plugin);
    _plugin = nullptr;
    if (_library != nullptr) {
        FreeLibrary(_library);
        _library = nullptr;
    }
}

bool ClapHost::canUseGui() const noexcept {
    if (!_pluginGui)
        return false;

    if (_pluginGui->is_api_supported && _pluginGui->get_preferred_api && _pluginGui->create &&
        _pluginGui->destroy && _pluginGui->set_scale && _pluginGui->get_size &&
        _pluginGui->can_resize && _pluginGui->get_resize_hints && _pluginGui->adjust_size &&
        _pluginGui->set_size && _pluginGui->set_parent && _pluginGui->set_transient &&
        _pluginGui->suggest_title && _pluginGui->show && _pluginGui->hide)
        return true;

    return false;
}

void ClapHost::start(double sampleRate, uint32_t bufferSize) {
    _sampleRate = sampleRate;
    _bufferSize = bufferSize;
    if (!_plugin) {
        return;
    }
    if (!_processing) {
        _plugin->activate(_plugin, sampleRate, bufferSize, bufferSize);
        _plugin->start_processing(_plugin);
        _processing = true;
    }
}

void ClapHost::stop() {
    if (!_plugin) {
        return;
    }
    if (_processing) {
        _plugin->stop_processing(_plugin);
        _plugin->deactivate(_plugin);
        _processing = false;
    }
}

void ClapHost::loadState() {
    auto path = _track->_composer->_project->projectDir() / _statePath;
    std::ifstream ofs(path, std::ios::binary);
    clap_istream stream{
        .ctx = &ofs,
        .read = [](const struct clap_istream* stream, void* buffer, uint64_t size) -> int64_t {
            ((std::ifstream*)stream->ctx)->read(static_cast<char*>(buffer), size);
            return ((std::ifstream*)stream->ctx)->gcount();
        }
    };
    _pluginState->load(_plugin, &stream);
}

void ClapHost::saveState() {
    auto path = _track->_composer->_project->projectDir() / _statePath;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream ofs(path, std::ios::binary);
    clap_ostream stream{
        .ctx = &ofs,
        .write = [](const struct clap_ostream* stream, const void* buffer, uint64_t size) -> int64_t {
            ((std::ofstream*)stream->ctx)->write(static_cast<const char*>(buffer), size);
            if (((std::ofstream*)stream->ctx)->fail()) {
                Error("ステート保存に失敗しました。");
            }
            return size;
        }
    };
    _pluginState->save(_plugin, &stream);
}

// TODO see host.hxx 130
const void* ClapHost::clapGetExtension(const clap_host_t* /* host */, const char* extension_id) noexcept {
    logger->debug("get extension {}", extension_id);
    if (!std::strcmp(extension_id, CLAP_EXT_AUDIO_PORTS)) {
        return &_hostAudioPorts;
    }
    if (!std::strcmp(extension_id, CLAP_EXT_LATENCY)) {
        return &_hostLatency;
    }

    return nullptr;
}

void ClapHost::clapRequestRestart(const clap_host_t* host) noexcept {
    logger->debug("request restart");
    ClapHost* pluginHost = (ClapHost*)host->host_data;
    auto plugin = pluginHost->_plugin;
    plugin->deactivate(plugin);
    plugin->activate(plugin, pluginHost->_sampleRate, pluginHost->_bufferSize, pluginHost->_bufferSize);
}

void ClapHost::clapRequestProcess(const clap_host_t* /* host */) noexcept {
    logger->debug("request process");
}

void ClapHost::clapRequestCallback(const clap_host_t* host) noexcept {
    logger->debug("request callback");
    std::lock_guard<std::mutex> lock(gClapRequestCallbackQueueMutex);
    gClapRequestCallbackQueue.push(host);
}

void ClapHost::changeLatency() {
    if (_pluginLatency != nullptr) {
        _latency = _pluginLatency->get(_plugin);
        if (_latency != 0) {
            logger->debug("Latency {}", _latency);
        }
    }
}

bool ClapHost::process(ProcessBuffer* buffer, int64_t steadyTime) {
    // とりあえず 0, 1 の 2ch で
    buffer->ensure32();
    _inputs[0] = buffer->_in[0].buffer32()[0].data();
    _inputs[1] = buffer->_in[0].buffer32()[1].data();
    _outputs[0] = buffer->_out[0].buffer32()[0].data();
    _outputs[1] = buffer->_out[0].buffer32()[1].data();

    _audioIn.channel_count = buffer->_in[0].getNchannels();
    _audioOut.channel_count = buffer->_in[0].getNchannels();

    _process.frames_count = buffer->_framesPerBuffer;
    _process.in_events = buffer->_eventIn.clapInputEvents();
    _process.out_events = _evOut.clapOutputEvents();
    _process.steady_time = steadyTime;

    if (_process.in_events->size(_process.in_events) > 0) {
        logger->debug("event");
    }
    try {
        clap_process_status status = _plugin->process(_plugin, &_process);
        if (status == CLAP_PROCESS_ERROR) {
            Error("Clap plugin process return error {}", status);
            return false;
        }
    } catch (const std::exception& e) {
        Error("Plack plugin render failed! {}", e.what());
        return false;
    } catch (...) {
        Error("Plack plugin render failed!\n\nUnknown error.");
        return false;
    }
    buffer->_out[0]._constantp.clear();
    for (uint32_t i = 0; i < _process.audio_outputs->channel_count; ++i) {
        buffer->_out[0]._constantp.push_back((_process.audio_outputs->constant_mask & (static_cast<unsigned long long>(1) << i)) != 0);
    }

    return true;
}

void ClapHost::openGui() {
    /// Showing the GUI works as follow:
    ///  1. clap_plugin_gui->is_api_supported(), check what can work
    ///  2. clap_plugin_gui->create(), allocates gui resources
    ///  3. if the plugin window is floating
    ///  4.    -> clap_plugin_gui->set_transient()
    ///  5.    -> clap_plugin_gui->suggest_title()
    ///  6. else
    ///  7.    -> clap_plugin_gui->set_scale()
    ///  8.    -> clap_plugin_gui->can_resize()
    ///  9.    -> if resizable and has known size from previous session, clap_plugin_gui->set_size()
    /// 10.    -> else clap_plugin_gui->get_size(), gets initial size
    /// 11.    -> clap_plugin_gui->set_parent()
    /// 12. clap_plugin_gui->show()
    /// 13. clap_plugin_gui->hide()/show() ...
    /// 14. clap_plugin_gui->destroy() when done with the gui

    auto api = CLAP_WINDOW_API_WIN32;
    auto is_floating = false;
    if (!canUseGui()) {
        return;
    }
    if (!_pluginGui->is_api_supported(_plugin, api, true)) {
        printf("is_api_supprted is_floating=true is false");
    }
    if (!_pluginGui->is_api_supported(_plugin, api, is_floating)) {
        printf("is_api_supprted is false");
        return;
    }
    _pluginGui->create(_plugin, api, is_floating);
    _pluginGui->set_scale(_plugin, 1);
    bool resizable = _pluginGui->can_resize(_plugin);
    uint32_t width, height;
    _pluginGui->get_size(_plugin, &width, &height);
    printf("can_resize %d, widht %d, height %d", resizable, width, height);
    if (!_gui) {
        _wndClass = WNDCLASSEXW{ sizeof(_wndClass), CS_CLASSDC, PluginHostWndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Plugin Window", nullptr };
        ::RegisterClassExW(&_wndClass);
        _hwnd = ::CreateWindowW(_wndClass.lpszClassName, L"plugin window", WS_OVERLAPPEDWINDOW, 300, 300, width + 14, height + 39, nullptr, nullptr, _wndClass.hInstance, this);
        ::ShowWindow(_hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(_hwnd);
        _clap_window.api = api;
        _clap_window.win32 = _hwnd;
        _gui = true;
    }
    _pluginGui->set_parent(_plugin, &_clap_window);
    if (!_pluginGui->show(_plugin)) {
        printf("show failed");
    }
}

void ClapHost::closeGui() {
    if (_gui) {
        _gui = false;
        _pluginGui->hide(_plugin);
        _pluginGui->destroy(_plugin);
        ::DestroyWindow(_hwnd);
        ::UnregisterClassW(_wndClass.lpszClassName, _wndClass.hInstance);
    }
}

LRESULT WINAPI PluginHostWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        ClapHost* pluginHost = (ClapHost*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pluginHost);
        break;
    }
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            // TODO なにかする
            printf("WM_SIZE\n");
        }
        return 0;
    case WM_DESTROY: {
        ClapHost* pluginHost = (ClapHost*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pluginHost->closeGui();
        break;
    }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
