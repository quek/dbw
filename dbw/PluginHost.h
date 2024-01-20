#pragma once

#include <clap/clap.h>
#include <clap/helpers/event-list.hh>
#include <clap/helpers/reducing-param-queue.hh>
#include <clap/helpers/host.hh>
#include <clap/helpers/plugin-proxy.hh>

// constexpr auto PluginHost_MH = clap::helpers::MisbehaviourHandler::Terminate;
// constexpr auto PluginHost_CL = clap::helpers::CheckingLevel::Maximal;

// using BaseHost = clap::helpers::Host<PluginHost_MH, PluginHost_CL>;
// extern template class clap::helpers::Host<PluginHost_MH, PluginHost_CL>;

// using PluginProxy = clap::helpers::PluginProxy<PluginHost_MH, PluginHost_CL>;
// extern template class clap::helpers::PluginProxy<PluginHost_MH, PluginHost_CL>;

class PluginHost
{
public:
	PluginHost(const clap_window* window);
	~PluginHost();
	bool load(const std::string path, uint32_t pluginIndex);
	clap_process* process(double sampleRate, uint32_t bufferSize);
	void edit();
	bool canUseGui() const noexcept;

private:
	const clap_window* _window;
	clap_host _clap_host;
	const clap_plugin* _plugin = nullptr;
	const clap_plugin_gui* _pluginGui = nullptr;
	bool _processing = false;
	// const clap_plugin_entry* _pluginEntry = nullptr;
	// const clap_plugin_factory* _pluginFactory = nullptr;
	// std::unique_ptr<PluginProxy> _plugin;

	/* process stuff */
	float* _inputs[2] = { nullptr, nullptr };
	float* _outputs[2] = { nullptr, nullptr };
	clap_process _process = {};
	clap::helpers::EventList _evIn;
	clap::helpers::EventList _evOut;
	// clap_process _process;


	static const void* clapGetExtension(const clap_host_t* host, const char* extension_id) noexcept;
	static void clapRequestRestart(const clap_host_t* host) noexcept;
	static void clapRequestProcess(const clap_host_t* host) noexcept;
	static void clapRequestCallback(const clap_host_t* host) noexcept;
};

