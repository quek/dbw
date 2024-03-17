#pragma once
#include <filesystem>
#include <nlohmann/json.hpp>

class ProcessBuffer;
class SerializeContext;

class AudioFile
{
public:
    AudioFile(const nlohmann::json& json, SerializeContext& context);
    AudioFile(const std::filesystem::path& _path);
    virtual ~AudioFile() = default;
    uint32_t copy(ProcessBuffer& processBuffer, int frameOffset, double start, double end, double oneBeatSec);
    float* getData() const { return _data.get(); }
    double getDuration(double bpm) const;
    uint32_t getNchannels() const { return _nchannels; }
    uint64_t getNframes() const { return _nframes; }
    const std::filesystem::path& getPath() const { return _path; }
    virtual nlohmann::json toJson(SerializeContext& context);

private:
    std::unique_ptr<float[]> _data = nullptr;
    uint64_t _nframes = 0;
    uint32_t _nchannels = 0;
    int _sampleRate = 0;
    std::filesystem::path _path;
};

