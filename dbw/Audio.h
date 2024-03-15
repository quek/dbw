#pragma once
#include <filesystem>
#include <memory>
#include "SequenceItem.h"
#include "Wav.h"


class Audio : public SequenceItem {
public:
    inline static const char* TYPE = "Audio";
    Audio(const nlohmann::json& json, SerializeContext& context);
    Audio(const std::filesystem::path& wavPath, double bpm);
    void prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec) override;
    virtual nlohmann::json toJson(SerializeContext& context) override;

private:
    std::filesystem::path _wavPath;
    std::unique_ptr<Wav> _wav;
};

