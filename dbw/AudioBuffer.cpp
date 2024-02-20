#include "AudioBuffer.h"
#include <ranges>
#include "logger.h"

AudioBuffer::AudioBuffer() : _framesPerBuffer(0), _nchannels(0) {
}

template<typename T, typename U>
void addBuffers(std::vector<std::vector<T>>& dest, const std::vector<std::vector<U>>& src, std::vector<bool>& destConstantp, const std::vector<bool>& srcConstantp) {
    for (auto [a, b, acp, bcp] : std::views::zip(dest, src, destConstantp, srcConstantp)) {
        for (auto [aa, bb] : std::views::zip(a, b)) {
            if (acp && bcp) {
                aa += static_cast<T>(bb);
                return;
            }
            if (acp) {
                aa = a[0] + static_cast<T>(bb);
            } else if (bcp) {
                aa += static_cast<T>(b[0]);
            } else {
                aa += static_cast<T>(bb);
            }
        }
    }
    std::ranges::fill(destConstantp, false);
}

template<typename T, typename U>
void copyBuffers(std::vector<std::vector<T>>& src, std::vector<std::vector<U>>& dest) {
    for (auto [a, b] : std::views::zip(src, dest)) {
        for (auto i = 0; i < a.size(); ++i) {
            b[i] = static_cast<U>(a[i]);
        }
    }
}

void AudioBuffer::add(const AudioBuffer& other) {
    if (_dataType == Float) {
        if (other._dataType == Float) {
            addBuffers(_buffer32, other._buffer32, _constantp, other._constantp);
        } else {
            addBuffers(_buffer32, other._buffer64, _constantp, other._constantp);
        }
    } else {
        if (other._dataType == Float) {
            addBuffers(_buffer64, other._buffer32, _constantp, other._constantp);
        } else {
            addBuffers(_buffer64, other._buffer64, _constantp, other._constantp);
        }
    }
}

void AudioBuffer::copyTo(AudioBuffer& other) {
    if (_dataType == Float) {
        if (other._dataType == Float) {
            copyBuffers(_buffer32, other._buffer32);
        } else {
            copyBuffers(_buffer32, other._buffer64);
        }
    } else {
        if (other._dataType == Float) {
            copyBuffers(_buffer64, other._buffer32);
        } else {
            copyBuffers(_buffer64, other._buffer64);
        }
    }
}

void AudioBuffer::copyFrom(const float** buffer, unsigned long framesPerBuffer, int nchannels) {
    ensure(framesPerBuffer, nchannels);
    for (int i = 0; i < nchannels; ++i) {
        _buffer32[i] = std::vector<float>(buffer[i], buffer[i] + framesPerBuffer);
    }
}

void AudioBuffer::copyFrom(const double** buffer, unsigned long framesPerBuffer, int nchannels) {
    ensure(framesPerBuffer, nchannels);
    for (int i = 0; i < nchannels; ++i) {
        _buffer64[i] = std::vector<double>(buffer[i], buffer[i] + framesPerBuffer);
    }
}

void AudioBuffer::copyTo(float** buffer, unsigned long framesPerBuffer, int nchannels) {
    if (_framesPerBuffer < framesPerBuffer || _nchannels < nchannels) {
        logger->error("AudioBuffer::copyTo float** failed!");
        return;
    }
    for (int i = 0; i < nchannels; ++i) {
        if (_constantp[i]) {
            std::fill_n(buffer[i], framesPerBuffer, _buffer32[i][0]);
        } else {
            std::ranges::copy(_buffer32[i], buffer[i]);
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
                if (_dataType == Float) {
                    buffer[i * nchannels + channel] = _buffer32[channel][0];
                } else {
                    buffer[i * nchannels + channel] = static_cast<float>(_buffer64[channel][0]);
                }
            } else {
                if (_dataType == Float) {
                    buffer[i * nchannels + channel] = _buffer32[channel][i];
                } else {
                    buffer[i * nchannels + channel] = static_cast<float>(_buffer64[channel][i]);
                }
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
    if (_dataType == Float) {
        _buffer32.clear();
        for (int i = 0; i < _nchannels; ++i) {
            _buffer32.push_back(std::vector<float>(framesPerBuffer, 0.0f));
        }
    } else {
        _buffer64.clear();
        for (int i = 0; i < _nchannels; ++i) {
            _buffer64.push_back(std::vector<double>(framesPerBuffer, 0.0));
        }
    }
    _constantp = std::vector<bool>(nchannels, false);
}

void AudioBuffer::ensure32() {
    if (_dataType == Float) {
        return;
    }
    _buffer32.clear();
    for (auto& x : _buffer64) {
        std::vector<float> yy;
        for (double xx : x) {
            yy.push_back(static_cast<float>(xx));
        }
        _buffer32.push_back(yy);
    }
    _dataType = Float;
}

void AudioBuffer::ensure64() {
    if (_dataType == Double) {
        return;
    }
    _buffer64.clear();
    for (auto& x : _buffer32) {
        std::vector<double> yy;
        for (float xx : x) {
            yy.push_back(xx);
        }
        _buffer64.push_back(yy);
    }
    _dataType = Double;
}

void AudioBuffer::zero(bool useConstant) {
    if (useConstant) {
        if (_dataType == Float) {
            for (auto& x : _buffer32) {
                x[0] = 0.0f;
            }
        } else {
            for (auto& x : _buffer64) {
                x[0] = 0.0;
            }
        }
        std::ranges::fill(_constantp, true);
    } else {
        if (_dataType == Float) {
            for (auto& x : _buffer32) {
                std::ranges::fill(x, 0.0f);
            }
        } else {
            for (auto& x : _buffer64) {
                std::ranges::fill(x, 0.0);
            }
        }
        std::ranges::fill(_constantp, false);
    }
}

std::vector<std::vector<float>>& AudioBuffer::buffer32() {
    ensure32();
    return _buffer32;
}

std::vector<std::vector<double>>& AudioBuffer::buffer64() {
    ensure64();
    return _buffer64;
}
