#include "AudioEngine.h"
#include <stdio.h>
#include <string.h>
#include "PluginHost.h"


/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/
static int paCallback(
	const void* /* inputBuffer */,
	void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* /* timeInfo */,
	PaStreamCallbackFlags /* statusFlags */,
	void* userData
) {
	/* Cast data passed through stream to our structure. */
	AudioEngine* audioEngine = (AudioEngine*)userData;
	float* out = (float*)outputBuffer;
	unsigned int i;

	clap_process* process = audioEngine->process(framesPerBuffer);
	if (process) {
		auto buffer = process->audio_outputs;
		bool isLeftConstant = (buffer->constant_mask & (static_cast<uint64_t>(1) << 0)) != 0;
		bool isRightConstant = (buffer->constant_mask & (static_cast<uint64_t>(1) << 1)) != 0;
		for (i = 0; i < framesPerBuffer; ++i) {
			*out = buffer->data32[0][isLeftConstant ? 0 : i];
			++out;
			*out = buffer->data32[1][isRightConstant ? 0 : i];
			++out;
		}
	}
	else {
		for (i = 0; i < framesPerBuffer; ++i) {
			*out = 0.0;
			++out;
			*out = 0.0;
			++out;
		}
	}

	return 0;
}

AudioEngine::AudioEngine()
{
	PaError err = Pa_Initialize();
	if (err != paNoError) {
		// TODO
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
	}
}

AudioEngine::~AudioEngine()
{
	stop();
	for (auto iterator = _pluginHosts.begin(); iterator != _pluginHosts.end(); ++iterator) {
		(*iterator)->unload();
		delete (*iterator);
	}
	_pluginHosts.clear();

	PaError err = Pa_Terminate();
	if (err != paNoError) {
		// TODO
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
	}
}

void AudioEngine::start()
{
	try {
		PaError err;

		const PaDeviceInfo* deviceInfo;
		PaDeviceIndex nDevices = Pa_GetDeviceCount();
		PaDeviceIndex deviceIndex = -1;
		for (PaDeviceIndex i = 0; i < nDevices; ++i) {
			deviceInfo = Pa_GetDeviceInfo(i);
			const char* name = deviceInfo->name;
			PaHostApiIndex apiIndex = deviceInfo->hostApi;
			const PaHostApiInfo* apiInfo = Pa_GetHostApiInfo(apiIndex);
			printf("%s, %s, %f\n", name, apiInfo->name, deviceInfo->defaultSampleRate);
			if (strcmp(name, "Prism Sound USB Audio Class 2.0") == 0 && strcmp(apiInfo->name, "ASIO") == 0) {
				deviceIndex = i;
				break;
			}
		}
		if (deviceIndex == -1) {
			// TODO
			printf("Device not found!\n");
		}

		PaStreamParameters inputParameters{
			 deviceIndex,
			 2,
			 paFloat32,
			 Pa_GetDeviceInfo(deviceIndex)->defaultLowInputLatency,
			 nullptr //See you specific host's API docs for info on using this field
		};
		PaStreamParameters outputParameters{};
		outputParameters.channelCount = 2;
		outputParameters.device = deviceIndex;
		outputParameters.hostApiSpecificStreamInfo = NULL;
		outputParameters.sampleFormat = paFloat32;
		outputParameters.suggestedLatency = Pa_GetDeviceInfo(deviceIndex)->defaultLowOutputLatency;
		outputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field

		err = Pa_IsFormatSupported(&inputParameters, &outputParameters, _sampleRate);
		if (err == paFormatIsSupported)
		{
			printf("Hooray!\n");
		}
		else
		{
			printf("Too Bad.\n");
		}

		err = Pa_OpenStream(
			&_stream,
			&inputParameters,
			&outputParameters,
			_sampleRate,
			_bufferSize,        /* frames per buffer, i.e. the number
			of sample frames that PortAudio will
			request from the callback. Many apps
			may want to use
			paFramesPerBufferUnspecified, which
			tells PortAudio to pick the best,
			possibly changing, buffer size.*/
			paNoFlag,
			paCallback, /* this is your callback function */
			(void*)this); /*This is a pointer that will be passed to your callback*/
		if (err != paNoError) {
			// TODO
			printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		}

		err = Pa_StartStream(_stream);
		if (err != paNoError) {
			// TODO
			printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		}
	}
	catch (...) {
		stop();
	}
}

void AudioEngine::stop()
{
	for (auto iterator = _pluginHosts.begin(); iterator != _pluginHosts.end(); ++iterator) {
		(*iterator)->stop();
	}

	PaError err;
	if (_stream != nullptr) {
		err = Pa_StopStream(_stream);
		if (err != paNoError) {
			// TODO
			printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		}
		err = Pa_CloseStream(_stream);
		if (err != paNoError) {
			// TODO
			printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		}
		_stream = nullptr;
	}
}

clap_process* AudioEngine::process(unsigned long framesPerBuffer)
{
	clap_process* process = nullptr;
	for (auto iterator = _pluginHosts.begin(); iterator != _pluginHosts.end(); ++iterator) {
		process = (*iterator)->process(_sampleRate, _bufferSize, _steadyTime);
	}
	_steadyTime += framesPerBuffer;
	return process;
}

PluginHost* AudioEngine::addPlugin(std::string path)
{
	PluginHost* pluginHost = new PluginHost();
	pluginHost->load(path.c_str(), 0);
	pluginHost->start(_sampleRate, _bufferSize);
	_pluginHosts.push_back(pluginHost);
	return pluginHost;
}

