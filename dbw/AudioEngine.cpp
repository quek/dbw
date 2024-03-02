#include "AudioEngine.h"
#include <stdio.h>
#include <string.h>
#include "App.h"
#include "Composer.h"
#include "Config.h"
#include "logger.h"
#include "Error.h"


/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/
static int paCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* /* timeInfo */,
    PaStreamCallbackFlags /* statusFlags */,
    void* userData
) {
    AudioEngine* audioEngine = (AudioEngine*)userData;
    audioEngine->_startTime = std::chrono::high_resolution_clock::now();

    int result = 0;
    try {
        std::lock_guard<std::recursive_mutex> lock(audioEngine->_app->_mtx);

        audioEngine->process((float*)inputBuffer, (float*)outputBuffer, framesPerBuffer);
        float* out = (float*)outputBuffer;
        for (unsigned long i = 0; i < framesPerBuffer * 2; ++i) {
            if (*out != 0.0) {
                if (*out > 1.0) {
                    *out = 1.0;
                }
            }
            ++out;
        }
    } catch (...) {
        logger->error("catch(...) in paCallback!");
        result = -1;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto processingTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - audioEngine->_startTime).count();
    double theoreticalProcessingTimeMs = (framesPerBuffer / gPreference.sampleRate) * 1000.0;
    double cpuLoad = (processingTimeMs / theoreticalProcessingTimeMs) * 100.0;
    audioEngine->_cpuLoad = cpuLoad;

    return result;
}

AudioEngine::AudioEngine(App* app) : _app(app) {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        // TODO
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }
}

AudioEngine::~AudioEngine() {
    stop();

    PaError err = Pa_Terminate();
    if (err != paNoError) {
        // TODO
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }
}

void AudioEngine::start() {
    if (_isStarted) {
        return;
    }
    if (gPreference.audioDeviceIndex == -1) {
        return;
    }
    if (Pa_GetDeviceInfo(gPreference.audioDeviceIndex) == nullptr) {
        gPreference.audioDeviceIndex = -1;
        return;
    }

    PaError err;
    PaStreamParameters inputParameters{
         gPreference.audioDeviceIndex,
         2,
         paFloat32,
         Pa_GetDeviceInfo(gPreference.audioDeviceIndex)->defaultLowInputLatency,
         nullptr
    };
    PaStreamParameters outputParameters{};
    outputParameters.channelCount = 2;
    outputParameters.device = gPreference.audioDeviceIndex;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(gPreference.audioDeviceIndex)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    err = Pa_OpenStream(
        &_stream,
        &inputParameters,
        &outputParameters,
        gPreference.sampleRate,
        gPreference.bufferSize,
        paNoFlag,
        paCallback,
        (void*)this);
    if (err != paNoError) {
        Error("PortAudio error: {}", Pa_GetErrorText(err));
        return;
    }

    err = Pa_StartStream(_stream);
    if (err != paNoError) {
        Error("PortAudio error: {}", Pa_GetErrorText(err));
        return;
    }
    _isStarted = true;
}

void AudioEngine::stop() {
    PaError err;
    if (_stream != nullptr) {
        err = Pa_StopStream(_stream);
        if (err != paNoError) {
            Error("PortAudio error: {}", Pa_GetErrorText(err));
        }
        err = Pa_CloseStream(_stream);
        if (err != paNoError) {
            Error("PortAudio error: {}", Pa_GetErrorText(err));
        }
        _stream = nullptr;
    }
    _isStarted = false;
}

void AudioEngine::process(float* in, float* out, unsigned long framesPerBuffer) {
    for (auto& composer : _app->composers()) {
        composer->process(in, out, framesPerBuffer, _steadyTime);
    }
    _steadyTime += framesPerBuffer;
}


