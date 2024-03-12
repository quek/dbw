#include "Wav.h"
#define DR_WAV_IMPLEMENTATION
#include <dr_libs/dr_wav.h>

Wav::Wav(const std::filesystem::path& file) {
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalPCMFrameCount;
    _data = drwav_open_file_and_read_pcm_frames_f32(file.string().c_str(), &channels, &sampleRate, &totalPCMFrameCount, nullptr);
    if (_data == nullptr) {
        // Error opening and reading WAV file.
    }
}

Wav::~Wav() {
    if (_data) {
        drwav_free(_data, nullptr);
        _data = nullptr;
    }
}
