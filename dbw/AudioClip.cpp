#include "AudioClip.h"
#include "Audio.h"

AudioClip::AudioClip(const nlohmann::json& json, SerializeContext& context) : Clip(json, context)
{
}

AudioClip::AudioClip(double time, const std::string& wavPath, double bpm) : Clip(time)
{
    Audio* audio = new Audio(wavPath, bpm);
    _sequence->addItem(audio);
    _duration = audio->_duration;
}

Clip* AudioClip::clone()
{
    return new AudioClip(*this);
}

void AudioClip::edit(Composer*, Lane*)
{
    // TODO
}

std::string AudioClip::name() const
{
    return "W" + Clip::name();
}

void AudioClip::renderInScene(PianoRollWindow*)
{
    // TODO
}

nlohmann::json AudioClip::toJson(SerializeContext& context)
{
    nlohmann::json json = Clip::toJson(context);
    json["type"] = TYPE;
    return json;
}
