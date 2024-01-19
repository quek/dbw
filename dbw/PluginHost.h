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
	PluginHost();
	~PluginHost();
	bool load(const std::string path, int pluginIndex);

private:
	clap_host _clap_host;
	// const clap_plugin_entry* _pluginEntry = nullptr;
	// const clap_plugin_factory* _pluginFactory = nullptr;
	// std::unique_ptr<PluginProxy> _plugin;

	/* process stuff */
	// clap_audio_buffer _audioIn = {};
	// clap_audio_buffer _audioOut = {};
	// clap::helpers::EventList _evIn;
	// clap::helpers::EventList _evOut;
	// clap_process _process;
};

