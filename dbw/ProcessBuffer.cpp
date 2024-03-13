#include "ProcessBuffer.h"

ProcessBuffer::ProcessBuffer() : _framesPerBuffer(0), _nbuses(0), _nchannels(0) {
}

void ProcessBuffer::ensure(unsigned long framesPerBuffer, unsigned int nbuses, unsigned int nchannels) {
    if (_framesPerBuffer == framesPerBuffer && _nbuses == nbuses && _nchannels == nchannels) {
        return;
    }
    _framesPerBuffer = framesPerBuffer;
    _nbuses = nbuses;
    _nchannels = nchannels;
    _in.clear();
    _out.clear();
    for (unsigned int i = 0; i < nbuses; ++i) {
        _in.emplace_back(AudioBuffer{});
        _out.emplace_back(AudioBuffer{});
    }
    for (auto& x : _in) {
        x.ensure(_framesPerBuffer, _nchannels);
    }
    for (auto& x : _out) {
        x.ensure(_framesPerBuffer, _nchannels);
    }
}

void ProcessBuffer::clear(bool useConstant) {
    inZero(useConstant);
    outZero(useConstant);
    _eventIn.clear();
    _eventOut.clear();
}

void ProcessBuffer::swapInOut() {
    std::swap(_in, _out);
    std::swap(_eventIn, _eventOut);
}

void ProcessBuffer::ensure32() {
    for (auto& x : _in) {
        x.ensure32();
    }
    for (auto& x : _out) {
        x.ensure32();
    }
}

void ProcessBuffer::inZero(bool useConstant) {
    for (auto& x : _in) {
        x.zero(useConstant);
    }
}

void ProcessBuffer::outZero(bool useConstant) {
    for (auto& x : _out) {
        x.zero(useConstant);
    }
}

void ProcessBuffer::ensure64() {
    for (auto& x : _in) {
        x.ensure64();
    }
    for (auto& x : _out) {
        x.ensure64();

    }
}
