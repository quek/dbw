#include "ProcessBuffer.h"

ProcessBuffer::ProcessBuffer() : ProcessBuffer(0) {
}

ProcessBuffer::ProcessBuffer(unsigned long framesPerBuffer) {
    ensure(framesPerBuffer);
}

void ProcessBuffer::ensure(unsigned long framesPerBuffer) {
    if (_framesPerBuffer != framesPerBuffer) {
        _framesPerBuffer = framesPerBuffer;
        _in.clear();
        _out.clear();
        for (auto i = 0; i < _nchanels; ++i) {
            _in.push_back(std::vector<float>(framesPerBuffer, 0.0f));
            _out.push_back(std::vector<float>(framesPerBuffer, 0.0f));
        }
    }
}

void ProcessBuffer::copyInToInFrom(const ProcessBuffer* from) {
    if (this == from) {
        return;
    }
    ensure(from->_framesPerBuffer);
    for (auto i = 0; i < _nchanels; ++i) {
        _in[i] = from->_in[i];
    }
}

void ProcessBuffer::copyOutToOutFrom(const ProcessBuffer* from) {
    if (this == from) {
        return;
    }
    ensure(from->_framesPerBuffer);
    for (auto i = 0; i < _nchanels; ++i) {
        _out[i] = from->_out[i];
    }
}

void ProcessBuffer::clear() {
    for (auto i = 0; i < _nchanels; ++i) {
        std::fill(_in[i].begin(), _in[i].end(), 0.0f);
        std::fill(_out[i].begin(), _out[i].end(), 0.0f);
    }
    _eventIn.clear();
    _eventOut.clear();
}

