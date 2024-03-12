#pragma once
#include <filesystem>

class Wav {
public:
    Wav(const std::filesystem::path& file);
    virtual ~Wav();
private:
    float* _data = nullptr;
};

