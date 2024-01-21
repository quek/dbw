#pragma once

#include <clap/clap.h>
#include <windows.h>
#include "PluginEventList.h"
#include <string>

class PluginHost
{
public:
	PluginHost();
	~PluginHost();
	bool load(const std::string path, uint32_t pluginIndex);
	void unload();
	clap_process* process(double sampleRate, uint32_t bufferSize, int64_t steadyTime);
	void openGui();
	void closeGui();
	bool canUseGui() const noexcept;
	void start(double sampleRate, uint32_t bufferSize);
	void stop();

private:
	bool _gui = false;
	clap_window _clap_window = {};
	HWND _hwnd = {};
	WNDCLASSEXW _wc = {};
	clap_host _clap_host;
	const clap_plugin* _plugin = nullptr;
	const clap_plugin_gui* _pluginGui = nullptr;
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


	static const void* clapGetExtension(const clap_host_t* host, const char* extension_id) noexcept;
	static void clapRequestRestart(const clap_host_t* host) noexcept;
	static void clapRequestProcess(const clap_host_t* host) noexcept;
	static void clapRequestCallback(const clap_host_t* host) noexcept;
};

