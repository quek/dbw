#pragma once

#include <clap/clap.h>
#include <portaudio.h>

class PluginHost;

class AudioEngine
{
public:
	AudioEngine(const clap_window* window);
	~AudioEngine();
	void start();
	void stop();
	clap_process* process(unsigned long framesPerBuffer);

private:
	const clap_window* _window;
	PaStream* _stream = nullptr;
	double _sampleRate = 48000.0;
	unsigned long _bufferSize = 1024;
	int64_t _steadyTime = 0;
	PluginHost* _pluginHost = nullptr;
};

