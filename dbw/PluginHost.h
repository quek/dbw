#pragma once

#include <clap/clap.h>
#include <clap/helpers/event-list.hh>
#include <clap/helpers/reducing-param-queue.hh>
#include <clap/helpers/host.hh>
#include <clap/helpers/plugin-proxy.hh>
#include <windows.h>

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
	bool load(const std::string path, uint32_t pluginIndex);
	clap_process* process(double sampleRate, uint32_t bufferSize, int64_t steadyTime);
	void openGui();
	void closeGui();
	bool canUseGui() const noexcept;
	void stop();

private:
	bool _gui = false;
	clap_window _clap_window;
	HWND _hwnd;
	WNDCLASSEXW _wc;
	clap_host _clap_host;
	const clap_plugin* _plugin = nullptr;
	const clap_plugin_gui* _pluginGui = nullptr;
	bool _processing = false;
	// const clap_plugin_entry* _pluginEntry = nullptr;
	// const clap_plugin_factory* _pluginFactory = nullptr;
	// std::unique_ptr<PluginProxy> _plugin;

	/* process stuff */
	unsigned long _allocatedSize = 0;
	float* _inputs[2] = { nullptr, nullptr };
	float* _outputs[2] = { nullptr, nullptr };
	clap_process _process = {};
	clap_audio_buffer _audioIn = {};
	clap_audio_buffer _audioOut = {};
	clap::helpers::EventList _evIn;
	clap::helpers::EventList _evOut;


	static const void* clapGetExtension(const clap_host_t* host, const char* extension_id) noexcept;
	static void clapRequestRestart(const clap_host_t* host) noexcept;
	static void clapRequestProcess(const clap_host_t* host) noexcept;
	static void clapRequestCallback(const clap_host_t* host) noexcept;
};

