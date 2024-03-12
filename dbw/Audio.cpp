#include "Audio.h"

Audio::Audio(const std::string& wavPath) : _wavPath(wavPath), _wav(new Wav(wavPath))
{

}
