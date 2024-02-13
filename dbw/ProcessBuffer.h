#pragma once
#include <memory>
#include "AudioBuffer.h"
#include "PluginEventList.h"

class ProcessBuffer {
public:
    ProcessBuffer();
    void ensure(unsigned long framesPerBuffer, int nchannels);
    void clear();
    void swapInOut();
    void ensure32();
    void ensure64();

    AudioBuffer _in;
    AudioBuffer _out;

    PluginEventList _eventIn;
    PluginEventList _eventOut;

    unsigned long _framesPerBuffer;
    int _nchannels = 2;
};
