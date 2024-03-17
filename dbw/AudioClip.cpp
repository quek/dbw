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

void AudioClip::render(const ImVec2& screenPos1, const ImVec2& screenPos2, const bool selected)
{
    Clip::render(screenPos1, screenPos2, selected);
    for (auto& audio : _sequence->getItems())
    {
        audio->render(screenPos1, screenPos2, selected);
    }
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
