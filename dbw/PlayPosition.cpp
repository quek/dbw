#include "PlayPosition.h"
#include <cmath>

PlayPosition PlayPosition::nextPlayPosition(double sampleRate, unsigned long framesPerBuffer, float bpm, int lpb) const
{
    double deltaSec = framesPerBuffer / sampleRate;
    double oneBeatSec = 60.0 / bpm;
    double oneLineSec = oneBeatSec / lpb;
    double quotient = std::floor(deltaSec / oneLineSec);
    double remainder = std::fmod(deltaSec, oneLineSec);
    int line = static_cast<int>(quotient) + _line;
    double oneDelaySec = oneLineSec / 0x100;
    auto delay = (std::floor(remainder / oneDelaySec)) + _delay;
    if (delay > 0xff) {
        ++line;
        delay -= 0x100;
    }

    return PlayPosition{ line,  static_cast<unsigned char>(delay) };
}

PlayPosition& PlayPosition::operator+=(const PlayPosition& rhs)
{
    _line += rhs._line;
    auto delay = _delay + rhs._delay;
    if (delay > 0xff) {
        _line += delay / 0x100;
        _delay = static_cast<unsigned char>(delay % 0x100);
    }
    else {
        _delay = static_cast<unsigned char>(delay);
    }
    return *this;
}

bool PlayPosition::operator>(const PlayPosition& other) const {
    return _line > other._line || (_line == other._line && _delay > other._delay);
}

bool PlayPosition::operator<(const PlayPosition& other) const {
    return _line < other._line || (_line == other._line && _delay < other._delay);
}

bool PlayPosition::operator>=(const PlayPosition& other) const {
    return !(*this < other);
}

bool PlayPosition::operator<=(const PlayPosition& other) const {
    return !(*this > other);
}

bool PlayPosition::operator==(const PlayPosition& other) const {
    return _line == other._line && _delay == other._delay;
}

bool PlayPosition::operator!=(const PlayPosition& other) const {
    return !(*this == other);
}

