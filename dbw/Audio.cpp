#include "Audio.h"
#include "Lane.h"
#include "ProcessBuffer.h"
#include "Track.h"

Audio::Audio(const std::string& wavPath) : _wavPath(wavPath), _wav(new Wav(wavPath))
{

}

void Audio::prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec)
{
    ProcessBuffer& processBuffer = lane->_track->_processBuffer;
    double wavBegin = begin - clipBegin;
    if (begin < end)
    {
        double wavEnd = wavBegin + (end - begin);
        _wav->copy(processBuffer, wavBegin, wavEnd, oneBeatSec);
    } else
    {
        // TODO
    }
}
