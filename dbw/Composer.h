#pragma once
#include "AudioEngine.h"
#include "PluginHost.h"

class Composer
{
public:
	Composer(AudioEngine* audioEngine);
	void render();

private:
	AudioEngine* _audioEngine;
	
	// delete
	PluginHost* _pluginHost = nullptr;;
	std::string _pluginPath = { "C:\\Program Files\\Common Files\\CLAP\\Surge Synth Team\\Surge XT.clap" };
};

