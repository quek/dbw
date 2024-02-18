#pragma once
#include <memory>
#include <vector>
#include "AudioBuffer.h"
#include "PluginEventList.h"

class ProcessBuffer {
public:
    ProcessBuffer();
    void ensure(unsigned long framesPerBuffer, int nbuses, int nchannels);
    void clear();
    void swapInOut();
    void ensure32();
    void ensure64();
    void inZero();
    void outZero();

    std::vector<AudioBuffer> _in;
    std::vector<AudioBuffer> _out;

    PluginEventList _eventIn;
    PluginEventList _eventOut;

    unsigned long _framesPerBuffer;
    int _nbuses;
    int _nchannels;
};
