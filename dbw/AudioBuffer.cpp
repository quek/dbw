#include "AudioBuffer.h"
#include "logger.h"

AudioBuffer::AudioBuffer() : _framesPerBuffer(0), _nchannels(0) {
}

void AudioBuffer::add(const AudioBuffer& other) {
    for (auto [a, b] = std::pair{ _buffer.begin(), other._buffer.begin() };
         a != _buffer.end() && b != other._buffer.end();
         ++a, ++b) {
        for (auto [aa, bb] = std::pair{ a->begin(), b->begin() };
             aa != a->end() && bb != b->end();
             ++aa, ++bb) {
            (*aa) += (*bb);
        }
    }
}

void AudioBuffer::copyFrom(const AudioBuffer& other) {
    _buffer = other._buffer;
    _constantp = other._constantp;
    _framesPerBuffer = other._framesPerBuffer;
    _nchannels = other._nchannels;
}

void AudioBuffer::copyFrom(const float** buffer, unsigned long framesPerBuffer, int nchannels) {
    ensure(framesPerBuffer, nchannels);
    for (int i = 0; i < nchannels; ++i) {
        _buffer[i] = std::vector<float>(buffer[i], buffer[i] + framesPerBuffer);
    }
}

void AudioBuffer::copyTo(float** buffer, unsigned long framesPerBuffer, int nchannels) {
    if (_framesPerBuffer < framesPerBuffer || _nchannels < nchannels) {
        logger->error("AudioBuffer::copyTo float** failed!");
        return;
    }
    for (int i = 0; i < nchannels; ++i) {
        if (_constantp[i]) {
            std::fill_n(buffer[i], framesPerBuffer, _buffer[i][0]);
        } else {
            std::copy(_buffer[i].begin(), _buffer[i].end(), buffer[i]);
        }
    }
}

void AudioBuffer::copyTo(float* buffer, unsigned long framesPerBuffer, int nchannels) {
    if (_framesPerBuffer < framesPerBuffer || _nchannels < nchannels) {
        logger->error("AudioBuffer::copyTo float* failed!");
        return;
    }
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        for (int channel = 0; channel < nchannels; ++channel) {
            if (_constantp[channel]) {
                buffer[i * nchannels + channel] = _buffer[channel][0];
            } else {
                buffer[i * nchannels + channel] = _buffer[channel][i];
            }
        }
    }
}

void AudioBuffer::ensure(unsigned long framesPerBuffer, int nchannels) {
    if (_framesPerBuffer == framesPerBuffer && _nchannels == nchannels) {
        return;
    }
    _framesPerBuffer = framesPerBuffer;
    _nchannels = nchannels;
    _buffer.clear();
    for (int i = 0; i < _nchannels; ++i) {
        _buffer.push_back(std::vector<float>(framesPerBuffer, 0.0f));
    }
    _constantp = std::vector<bool>(nchannels, false);
}

void AudioBuffer::zero() {
    for (int i = 0; i < _nchannels; ++i) {
        std::fill(_buffer[i].begin(), _buffer[i].end(), 0.0f);
    }
    std::fill(_constantp.begin(), _constantp.end(), false);
}

