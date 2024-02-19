#include "AudioEngine.h"
#include <stdio.h>
#include <string.h>
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
    /* Cast data passed through stream to our structure. */
    AudioEngine* audioEngine = (AudioEngine*)userData;
    int result = 0;
    try {
        std::lock_guard<std::mutex> lock(audioEngine->mtx);

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

    return result;
}

AudioEngine::AudioEngine() {
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
    }

    err = Pa_StartStream(_stream);
    if (err != paNoError) {
        Error("PortAudio error: {}", Pa_GetErrorText(err));
    }
    _isStarted = true;
}

//void AudioEngine::start() {
//    if (_isStarted) {
//        return;
//    }
//    try {
//        PaError err;
//
//        const PaDeviceInfo* deviceInfo;
//        PaDeviceIndex nDevices = Pa_GetDeviceCount();
//        for (PaDeviceIndex i = 0; i < nDevices; ++i) {
//            deviceInfo = Pa_GetDeviceInfo(i);
//            const char* name = deviceInfo->name;
//            PaHostApiIndex apiIndex = deviceInfo->hostApi;
//            const PaHostApiInfo* apiInfo = Pa_GetHostApiInfo(apiIndex);
//            printf("%s, %s, %f\n", name, apiInfo->name, deviceInfo->defaultSampleRate);
//            if (strcmp(name, "Prism Sound USB Audio Class 2.0") == 0 && strcmp(apiInfo->name, "ASIO") == 0) {
//                _deviceIndex = i;
//                break;
//            }
//        }
//        if (_deviceIndex == -1) {
//            // TODO
//            printf("Device not found!\n");
//        }
//
//        PaStreamParameters inputParameters{
//             _deviceIndex,
//             2,
//             paFloat32,
//             Pa_GetDeviceInfo(_deviceIndex)->defaultLowInputLatency,
//             nullptr //See you specific host's API docs for info on using this field
//        };
//        PaStreamParameters outputParameters{};
//        outputParameters.channelCount = 2;
//        outputParameters.device = _deviceIndex;
//        outputParameters.hostApiSpecificStreamInfo = NULL;
//        outputParameters.sampleFormat = paFloat32;
//        outputParameters.suggestedLatency = Pa_GetDeviceInfo(_deviceIndex)->defaultLowOutputLatency;
//        outputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
//
//        err = Pa_IsFormatSupported(&inputParameters, &outputParameters, _sampleRate);
//        if (err == paFormatIsSupported) {
//            printf("Hooray!\n");
//        } else {
//            printf("Too Bad.\n");
//        }
//
//        err = Pa_OpenStream(
//            &_stream,
//            &inputParameters,
//            &outputParameters,
//            _sampleRate,
//            _bufferSize,        /* frames per buffer, i.e. the number
//            of sample frames that PortAudio will
//            request from the callback. Many apps
//            may want to use
//            paFramesPerBufferUnspecified, which
//            tells PortAudio to pick the best,
//            possibly changing, buffer size.*/
//            paNoFlag,
//            paCallback, /* this is your callback function */
//            (void*)this); /*This is a pointer that will be passed to your callback*/
//        if (err != paNoError) {
//            // TODO
//            printf("PortAudio error: %s\n", Pa_GetErrorText(err));
//        }
//
//        err = Pa_StartStream(_stream);
//        if (err != paNoError) {
//            // TODO
//            printf("PortAudio error: %s\n", Pa_GetErrorText(err));
//        }
//        _isStarted = true;
//    } catch (...) {
//        stop();
//    }
//}

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
    _composer->process(in, out, framesPerBuffer, _steadyTime);
    _steadyTime += framesPerBuffer;
}


