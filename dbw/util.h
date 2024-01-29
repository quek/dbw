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
std::string GetExecutablePath();
std::filesystem::path getOpenFileName();
std::filesystem::path projectDir();
std::string yyyyMmDdHhMmSs();
std::filesystem::path userDir();
