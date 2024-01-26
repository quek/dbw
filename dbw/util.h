#pragma once
#include <queue>
#include <string>
#include <mutex>
#include <clap/clap.h>

std::string GetExecutablePath();
extern std::queue<const clap_host*> gClapRequestCallbackQueue;
extern std::mutex gClapRequestCallbackQueueMutex;
