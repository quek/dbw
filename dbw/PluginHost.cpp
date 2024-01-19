#include "PluginHost.h"

#include <windows.h>

PluginHost::PluginHost() {
	_clap_host = clap_host {
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
		"https://github.com/quek/dbw"
		// const char *version; // eg: "4.3", see plugin.h for advice on how to format the version
		"0.0.1",
		// Query an extension.
		// The returned pointer is owned by the host.
		// It is forbidden to call it before plugin->init().
		// You can call it within plugin->init() call, and after.
		// [thread-safe]
		// const void *(CLAP_ABI *get_extension)(const struct clap_host *host, const char *extension_id);
		nullptr,
		// Request the host to deactivate and then reactivate the plugin.
		// The operation may be delayed by the host.
		// [thread-safe]
		// void(CLAP_ABI *request_restart)(const struct clap_host *host);
		nullptr,
		// Request the host to activate and start processing the plugin.
		// This is useful if you have external IO and need to wake up the plugin from "sleep".
		// [thread-safe]
		// void(CLAP_ABI *request_process)(const struct clap_host *host);
		nullptr,
		// Request the host to schedule a call to plugin->on_main_thread(plugin) on the main thread.
		// [thread-safe]
		// void(CLAP_ABI *request_callback)(const struct clap_host *host);
		nullptr,
	};
}

PluginHost::~PluginHost() {}

bool PluginHost::load(const std::string path, int pluginIndex)
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

	const auto plugin = factory->create_plugin(factory, &_clap_host, desc->id);
	if (!plugin) {
		printf("could not create the plugin with id: %s", desc->id);
		return false;
	}

	if (!plugin->init(plugin)) {
		printf("could not init the plugin with id: %s", desc->id);
		return false;
	}
	return true;
}
