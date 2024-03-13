#pragma once
#include <deque>
#include <memory>
#include <vector>
#include "AudioBuffer.h"
#include "PluginEventList.h"

class ProcessBuffer {
public:
    ProcessBuffer();
    void ensure(unsigned long framesPerBuffer, unsigned int nbuses, unsigned int nchannels);
    void clear(bool useConstant = true);
    void swapInOut();
    void ensure32();
    void ensure64();
    void inZero(bool useConstant = true);
    void outZero(bool useConstant = true);

    std::vector<AudioBuffer> _in;
    std::vector<AudioBuffer> _out;

    std::vector<std::deque<float>> _dcpBuffer;

    PluginEventList _eventIn;
    PluginEventList _eventOut;

    unsigned long _framesPerBuffer;
    unsigned int _nbuses;
    unsigned int _nchannels;
};
