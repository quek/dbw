#include "PluginHost.h"

#include <windows.h>


PluginHost::PluginHost(const clap_window* window) : _window(window) {
	_clap_host = clap_host{
		// clap_version_t clap_version; // initialized to CLAP_VERSION
		CLAP_VERSION,
		// void *host_data; // reserved pointer for the host
		nullptr,
		// name and version are mandatory.
		// const char *name;    // eg: "Bitwig Studio"
		"dbw",
		// const char *vendor;  // eg: "Bitwig GmbH"
		"dbw",
		// const char *url;     // eg: "https://bitwig.com"
		"https://github.com/quek/dbw",
		// const char *version; // eg: "4.3", see plugin.h for advice on how to format the version
		"0.0.1",
		// Query an extension.
		// The returned pointer is owned by the host.
		// It is forbidden to call it before plugin->init().
		// You can call it within plugin->init() call, and after.
		// [thread-safe]
		// const void *(CLAP_ABI *get_extension)(const struct clap_host *host, const char *extension_id);
		clapGetExtension,
		// Request the host to deactivate and then reactivate the plugin.
		// The operation may be delayed by the host.
		// [thread-safe]
		// void(CLAP_ABI *request_restart)(const struct clap_host *host);
		clapRequestRestart,
		// Request the host to activate and start processing the plugin.
		// This is useful if you have external IO and need to wake up the plugin from "sleep".
		// [thread-safe]
		// void(CLAP_ABI *request_process)(const struct clap_host *host);
		clapRequestProcess,
		// Request the host to schedule a call to plugin->on_main_thread(plugin) on the main thread.
		// [thread-safe]
		// void(CLAP_ABI *request_callback)(const struct clap_host *host);
		clapRequestCallback,
	};

	// Set to -1 if not available, otherwise the value must be greater or equal to 0,
	// and must be increased by at least `frames_count` for the next call to process.
	// int64_t steady_time;
	_process.steady_time = 0;
	// time info at sample 0
	// If null, then this is a free running host, no transport events will be provided
	_process.transport = nullptr;
	_process.audio_inputs_count = 2;
	_process.audio_outputs_count = 2;

	_audioIn.data32 = _inputs;
	_audioIn.data64 = nullptr;
	_audioIn.channel_count = 2;
	_audioIn.latency = 0;
	_audioIn.constant_mask = 0;
	_process.audio_inputs = &_audioIn;
	_audioOut.data32 = _inputs;
	_audioOut.data64 = nullptr;
	_audioOut.channel_count = 2;
	_audioOut.latency = 0;
	_audioOut.constant_mask = 0;
	_process.audio_outputs = &_audioOut;
}

PluginHost::~PluginHost() {}

bool PluginHost::load(const std::string path, uint32_t pluginIndex)
{
	std::wstring wstr(path.begin(), path.end());
	LPCWSTR lpwstr = wstr.c_str();
	HMODULE library = LoadLibrary(lpwstr);
	if (!library) {
		// TODO
		printf("Load error %s", path.c_str());
		return false;
	}
	struct clap_plugin_entry* entry = (struct clap_plugin_entry*)GetProcAddress(library, "clap_entry");
	entry->init(path.c_str());
	auto factory =
		static_cast<const clap_plugin_factory*>(entry->get_factory(CLAP_PLUGIN_FACTORY_ID));
	auto count = factory->get_plugin_count(factory);
	if (pluginIndex > count) {
		// TODO
		printf("pluginIndex %d > count %d is false", pluginIndex, count);
		return false;
	}

	auto desc = factory->get_plugin_descriptor(factory, pluginIndex);
	if (!desc) {
		printf("no plugin descriptor");
		return false;
	}

	if (!clap_version_is_compatible(desc->clap_version)) {
		printf("Incompatible clap version");
		return false;
	}

	_plugin = factory->create_plugin(factory, &_clap_host, desc->id);
	if (!_plugin) {
		printf("could not create the plugin with id: %s", desc->id);
		return false;
	}

	if (!_plugin->init(_plugin)) {
		printf("could not init the plugin with id: %s", desc->id);
		return false;
	}



	_pluginGui = static_cast<const clap_plugin_gui*>(_plugin->get_extension(_plugin, CLAP_EXT_GUI));

	return true;
}

bool PluginHost::canUseGui() const noexcept {
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

void PluginHost::stop()
{
	if (!_plugin) {
		return;
	}
	if (_processing) {
		_plugin->stop_processing(_plugin);
		_plugin->deactivate(_plugin);
		_processing = false;
	}
	_plugin->destroy(_plugin);
}

const void* PluginHost::clapGetExtension(const clap_host_t* host, const char* extension_id) noexcept
{
	printf("get extension %s\n", extension_id);
	return nullptr;
}

void PluginHost::clapRequestRestart(const clap_host_t* /* host */) noexcept
{
	printf("request restart\n");
}

void PluginHost::clapRequestProcess(const clap_host_t* /* host */) noexcept
{
	printf("request process\n");
}

void PluginHost::clapRequestCallback(const clap_host_t* /* host */) noexcept
{
	printf("request callback\n");
}

clap_process* PluginHost::process(double sampleRate, uint32_t bufferSize, int64_t steadyTime) {
	if (!_processing) {
		_plugin->activate(_plugin, sampleRate, bufferSize, bufferSize);
		_plugin->start_processing(_plugin);
		_processing = true;
	}
	if (_process.frames_count != bufferSize) {
		_process.frames_count = bufferSize;
		if (_inputs[0] != nullptr) {
			std::free(_inputs[0]);
		}
		_inputs[0] = (float*)std::calloc(1, bufferSize);
		if (_inputs[1] != nullptr) {
			std::free(_inputs[1]);
		}
		_inputs[1] = (float*)std::calloc(1, bufferSize);

		if (_outputs[0] != nullptr) {
			std::free(_outputs[0]);
		}
		_outputs[0] = (float*)std::calloc(1, bufferSize);
		if (_outputs[1] != nullptr) {
			std::free(_outputs[1]);
		}
		_outputs[1] = (float*)std::calloc(1, bufferSize);
	}
	_process.in_events = _evIn.clapInputEvents();
	_process.out_events = _evOut.clapOutputEvents();
	_process.steady_time = steadyTime;

	clap_process_status status = _plugin->process(_plugin, &_process);
	if (status == CLAP_PROCESS_ERROR) {
		printf("process error");
	}

	return &_process;
}

void PluginHost::edit()
{
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

	auto api = _window->api;
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
	_pluginGui->set_parent(_plugin, _window);
	if (!_pluginGui->show(_plugin)) {
		printf("show failed");
	}
}
