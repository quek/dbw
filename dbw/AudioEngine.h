#pragma once

#include <clap/clap.h>
#include <portaudio.h>

class AudioEngine
{
public:
	AudioEngine();
	~AudioEngine();
	void start();
	void stop();

	// TODO delete 
	float left_phase = 0.0;
	float right_phase = 0.0;

private:
	PaStream* _stream = nullptr;
	double _sampleRate = 48000.0;
	unsigned long _bufferSize = 1024;
};

