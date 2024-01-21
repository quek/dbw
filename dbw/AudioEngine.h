#pragma once

#include <clap/clap.h>
#include <portaudio.h>
#include <string>
#include <vector>

class PluginHost;

class AudioEngine
{
public:
	AudioEngine();
	~AudioEngine();
	void start();
	void stop();
	clap_process* process(unsigned long framesPerBuffer);

	std::vector<PluginHost*> _pluginHosts;
	PluginHost* addPlugin(std::string path);
private:
	PaStream* _stream = nullptr;
	double _sampleRate = 48000.0;
	unsigned long _bufferSize = 1024;
	int64_t _steadyTime = 0;
};

