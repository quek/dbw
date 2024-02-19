#include "ProcessBuffer.h"

ProcessBuffer::ProcessBuffer() : _framesPerBuffer(0), _nbuses(0), _nchannels(0), _latency(0) {
    setLatency(0);
}

void ProcessBuffer::ensure(unsigned long framesPerBuffer, int nbuses, int nchannels) {
    if (_framesPerBuffer >= framesPerBuffer && _nbuses >= nbuses && _nchannels >= nchannels) {
        return;
    }
    _framesPerBuffer = framesPerBuffer;
    _nchannels = nchannels;
    for (int i = _nbuses; i < nbuses; ++i) {
        _in.emplace_back(AudioBuffer{});
        _out.emplace_back(AudioBuffer{});
    }
    _nbuses = nbuses;
    for (auto& x : _in) {
        x.ensure(_framesPerBuffer, _nchannels);
    }
    for (auto& x : _out) {
        x.ensure(_framesPerBuffer, _nchannels);
    }
}

void ProcessBuffer::clear() {
    inZero();
    outZero();
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

void ProcessBuffer::inZero() {
    for (auto& x : _in) {
        x.zero();
    }
}

void ProcessBuffer::outZero() {
    for (auto& x : _out) {
        x.zero();
    }
}

void ProcessBuffer::setLatency(uint32_t latency) {
    _latency = latency;
    if (_latency == 0) {
        return;
    }
    _dcpBuffer.clear();
    for (int i = 0; i < 2; ++i) {
        _dcpBuffer.emplace_back(latency, 0.0f);
    }
}

void ProcessBuffer::doDCP() {
    if (_latency == 0) {
        return;
    }
    for (int channel = 0; channel < 2; ++channel) {
        auto& out = _out[0].buffer32()[channel];
        auto& dcp = _dcpBuffer[channel];
        for (unsigned long i = 0; i < _framesPerBuffer; ++i) {
            dcp.push_back(out[i]);
            out[i] = dcp.front();
            dcp.pop_front();
        }
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
