#include "util.h"
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <windows.h>
#include <commdlg.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Comdlg32.lib")

std::queue<const clap_host*> gClapRequestCallbackQueue;
std::mutex gClapRequestCallbackQueueMutex;

std::filesystem::path configDir() {
    auto dir = userDir() / "config";
    std::filesystem::create_directories(dir);
    return dir;
}

std::string generateUniqueId() {
    // 現在のタイムスタンプ取得
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // ランダムな数値生成
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1000000, 9999999); // 範囲指定

    // IDを生成
    std::stringstream ss;
    ss << timestamp << "-" << dist(rng);

    return ss.str();
}

std::string GetExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    PathRemoveFileSpecA(path);
    return std::string(path);
}

std::filesystem::path getOpenFileName() {
    OPENFILENAME ofn;
    wchar_t szFile[1024] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        return szFile;
    }

    return std::filesystem::path();
}

std::filesystem::path projectDir() {
    return userDir() / "projects";
}

std::string yyyyMmDd() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_c);
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y%m%d");
    return ss.str();
}

std::string yyyyMmDdHhMmSs() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_c);
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y/%m/%d %H:%M:%S");
    return ss.str();
}

std::filesystem::path userDir() {
    return std::filesystem::path(GetExecutablePath()) / "user";
}

std::filesystem::path systemDir() {
    return std::filesystem::path(GetExecutablePath()) / "system";
}

std::string WideStringToAnsiString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

