#pragma once
#include <filesystem>
#include <queue>
#include <string>
#include <mutex>
#include <clap/clap.h>

extern std::queue<const clap_host*> gClapRequestCallbackQueue;
extern std::mutex gClapRequestCallbackQueueMutex;

std::filesystem::path configDir();
std::string generateUniqueId();
std::filesystem::path GetExecutablePath();
std::string yyyyMmDd();
std::string yyyyMmDdHhMmSs();
std::filesystem::path userDir();
std::filesystem::path systemDir();
std::string WideStringToAnsiString(const std::wstring& wstr);
std::wstring AnsiStringToWideString(const std::string& str);
