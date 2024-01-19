#include "AudioEngine.h"
#include <stdio.h>
#include <string.h>


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

	for (i = 0; i < framesPerBuffer; i++)
	{
		*out = audioEngine->left_phase * 0.2f;  /* left */
		++out;
		*out = audioEngine->right_phase * 0.2f;  /* right */
		++out;
		/* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
		audioEngine->left_phase += 0.01f;
		/* When signal reaches top, drop back down. */
		if (audioEngine->left_phase >= 1.0f) audioEngine->left_phase -= 2.0f;
		/* higher pitch so we can distinguish left and right. */
		audioEngine->right_phase += 0.03f;
		if (audioEngine->right_phase >= 1.0f) audioEngine->right_phase -= 2.0f;
	}
	return 0;
}

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
}

void AudioEngine::start()
{
	try {
		PaError err = Pa_Initialize();
		if (err != paNoError) {
			// TODO
			printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		}

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
	}

	err = Pa_Terminate();
	if (err != paNoError) {
		// TODO
		printf("PortAudio error: %s\n", Pa_GetErrorText(err));
	}
}


