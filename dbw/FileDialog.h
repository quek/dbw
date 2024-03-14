#pragma once
#include <filesystem>
#include <string>
#include <vector>

constexpr int MAX_PATH_LONG = 32767;

class FileDialog
{
public:
    static std::pair<bool, std::filesystem::path> getOpenFileName(
        const std::filesystem::path& initDir,
        const std::vector<std::pair<std::wstring, std::wstring>> extensions = { {L"JSON(*.json)", L"*.json"}, { L"All(*.*)", L"*.*" } });
    static std::pair<bool, std::filesystem::path> getSaveFileName(
        const std::filesystem::path& initPath,
        const std::vector<std::pair<std::wstring, std::wstring>> extensions = { {L"JSON(*.json)", L"*.json"}, { L"All(*.*)", L"*.*" } });
};

