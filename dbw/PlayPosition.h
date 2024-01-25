#pragma once

class PlayPosition {
public:
    int _line = 0;
    unsigned char _delay = 0;

    //int bar = 0;
    //int beat = 0;
    //int tick = 0;
    //static const int TICK_PER_BEAT = 960;

    PlayPosition nextPlayPosition(double sampleRate, unsigned long framesPerBuffer, float bpm, int lpb, int* samplePerDelay) const;
    int diffInDelay(const PlayPosition& other) const;
    PlayPosition& operator+=(const PlayPosition& rhs);
    bool operator>(const PlayPosition& other) const;
    bool operator<(const PlayPosition& other) const;
    bool operator>=(const PlayPosition& other) const;
    bool operator<=(const PlayPosition& other) const;
    bool operator==(const PlayPosition& other) const;
    bool operator!=(const PlayPosition& other) const;
};

