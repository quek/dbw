#pragma once
#include <memory>
#include "PluginEventList.h"

class ProcessBuffer {
public:
    ProcessBuffer();
    ProcessBuffer(unsigned long framesPerBuffer);
    void ensure(unsigned long framesPerBuffer);
    std::vector<std::vector<float>> _in;
    std::vector<std::vector<float>> _out;
    unsigned long _framesPerBuffer;
    void copyInToInFrom(const ProcessBuffer* from);
    void copyOutToOutFrom(const ProcessBuffer* from);
    PluginEventList _eventIn;
    PluginEventList _eventOut;
    int _nchanels = 2;

    void clear();
};

