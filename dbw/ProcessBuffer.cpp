#include "ProcessBuffer.h"

ProcessBuffer::ProcessBuffer() : _framesPerBuffer(0), _nchannels(0) {
}

void ProcessBuffer::ensure(unsigned long framesPerBuffer, int nchannels) {
    if (_framesPerBuffer == framesPerBuffer && _nchannels == nchannels) {
        return;
    }
    _framesPerBuffer = framesPerBuffer;
    _nchannels = nchannels;
    _in.ensure(_framesPerBuffer, _nchannels);
    _out.ensure(_framesPerBuffer, _nchannels);
}

void ProcessBuffer::clear() {
    _in.zero();
    _out.zero();
    _eventIn.clear();
    _eventOut.clear();
}

void ProcessBuffer::swapInOut() {
    std::swap(_in, _out);
    std::swap(_eventIn, _eventOut);
}

