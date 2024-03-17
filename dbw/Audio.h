#pragma once
#include <filesystem>
#include <memory>
#include "SequenceItem.h"
#include "AudioFile.h"


class Audio : public SequenceItem {
public:
    inline static const char* TYPE = "Audio";
    Audio(const nlohmann::json& json, SerializeContext& context);
    Audio(const std::filesystem::path& path, double bpm);
    void prepareProcessBuffer(Lane* lane, double begin, double end, double clipBegin, double clipEnd, double loopBegin, double loopEnd, double oneBeatSec) override;
    void render(const ImVec2& screenPos1, const ImVec2& screenPos2, const ImVec2& canvasPos1, const ImVec2& canvasPos2, const bool selected) override;
    virtual nlohmann::json toJson(SerializeContext& context) override;

private:
    std::unique_ptr<AudioFile> _audioFile;
};

