#include "Audio.h"
#include "Lane.h"
#include "ProcessBuffer.h"
#include "Track.h"

Audio::Audio(const std::string& wavPath, double bpm) : _wavPath(wavPath), _wav(new Wav(wavPath))
{
    _duration = _wav->getDuration(bpm);
}

void Audio::prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec)
{
    ProcessBuffer& processBuffer = lane->_track->_processBuffer;
    double wavBegin = begin - clipBegin;
    if (begin < end || loopEnd <= begin)
    {
        double wavEnd = wavBegin + (end - begin);
        _wav->copy(processBuffer, 0, wavBegin, wavEnd, oneBeatSec);
    }
    else
    {
        double duration = loopEnd - begin;
        double wavEnd = wavBegin + duration;
        uint32_t frameOffset = _wav->copy(processBuffer, 0, wavBegin, wavEnd, oneBeatSec);
        wavBegin = 0.0;
        wavEnd = duration;
        _wav->copy(processBuffer, frameOffset, wavBegin, wavEnd, oneBeatSec);
    }
}
