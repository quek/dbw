#include "util.h"
#include <Windows.h>

// PathRemoveFileSpecA
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

std::string GetExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    PathRemoveFileSpecA(path);
    return std::string(path);
}

std::queue<const clap_host*> gClapRequestCallbackQueue;
std::mutex gClapRequestCallbackQueueMutex;
