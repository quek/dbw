#include "AudioClip.h"
#include "Audio.h"

AudioClip::AudioClip(double time, const std::string& wavPath) : Clip(time)
{
    Audio* audio = new Audio(wavPath);
    _sequence->addItem(audio);
}

Clip* AudioClip::clone()
{
    return new AudioClip(*this);
}

void AudioClip::edit(Composer* , Lane* )
{
    // TODO
}

std::string AudioClip::name() const
{
    return "W" + Clip::name();
}

void AudioClip::renderInScene(PianoRollWindow* )
{
    // TODO
}
