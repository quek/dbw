#pragma once
#include "AudioEngine.h"
#include "PluginHost.h"

class MainWindow
{
public:
	MainWindow(AudioEngine* audioEngine);
	void render();

private:
	AudioEngine* _audioEngine;
	
	// delete
	PluginHost* _pluginHost = nullptr;;
};

