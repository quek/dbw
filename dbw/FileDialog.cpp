#include "FileDialog.h"
#include <cstring>
#include <windows.h>

std::pair<bool, std::filesystem::path> FileDialog::getOpenFileName(const std::filesystem::path& initDir, const std::vector<std::pair<std::wstring, std::wstring>> extensions)
{
    std::wstring initDirStr = initDir.wstring();

    std::wostringstream filter;
    for (auto& [a, b] : extensions)
    {
        filter << a << L'\0' << b << L'\0';
    }
    filter << L'\0';
    std::wstring filterStr = filter.str();

    std::unique_ptr<TCHAR[]> szFile(new TCHAR[MAX_PATH_LONG]());

    OPENFILENAME arg{
        .lStructSize = {sizeof(arg)},
        .hwndOwner = {},
        .hInstance = {},
        .lpstrFilter = {filterStr.c_str()},
        .lpstrCustomFilter = {},
        .nMaxCustFilter = {},
        .nFilterIndex = {1},
        .lpstrFile = {szFile.get()},
        .nMaxFile = {MAX_PATH_LONG},
        .lpstrFileTitle = {},
        .nMaxFileTitle = {},
        .lpstrInitialDir = {initDirStr.c_str()},
        .lpstrTitle = {L"ファイル選択"},
        .Flags = {OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST},
        .nFileOffset = {},
        .nFileExtension = {},
        .lpstrDefExt = {},
        .lCustData = {},
        . lpfnHook = {},
        .lpTemplateName = {},
        .pvReserved = {},
        .dwReserved = {},
        .FlagsEx = {},
    };

    if (GetOpenFileName(&arg) == TRUE)
    {
        return { true, std::filesystem::path(szFile.get()) };
    }
    return { false, std::filesystem::path() };
}

std::pair<bool, std::filesystem::path> FileDialog::getSaveFileName(const std::filesystem::path& initPath, const std::vector<std::pair<std::wstring, std::wstring>> extensions)
{
    std::wstring initPathStr = initPath.wstring();
    std::wstring initDirStr = initPath.parent_path().wstring();

    std::wostringstream filter;
    for (auto& [a, b] : extensions)
    {
        filter << a << L'\0' << b << L'\0';
    }
    filter << L'\0';
    std::wstring filterStr = filter.str();

    std::unique_ptr<TCHAR[]> szFile(new TCHAR[MAX_PATH_LONG]());
    wcsncpy_s(szFile.get(), MAX_PATH_LONG, initPathStr.c_str(), _TRUNCATE);

    OPENFILENAME arg{
        .lStructSize = {sizeof(arg)},
        .hwndOwner = {},
        .hInstance = {},
        .lpstrFilter = {filterStr.c_str()},
        .lpstrCustomFilter = {},
        .nMaxCustFilter = {},
        .nFilterIndex = {1},
        .lpstrFile = {szFile.get()},
        .nMaxFile = {MAX_PATH_LONG},
        .lpstrFileTitle = {},
        .nMaxFileTitle = {},
        .lpstrInitialDir = {initDirStr.c_str()},
        .lpstrTitle = {L"ファイル選択"},
        .Flags = {OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT},
        .nFileOffset = {},
        .nFileExtension = {},
        .lpstrDefExt = {},
        .lCustData = {},
        . lpfnHook = {},
        .lpTemplateName = {},
        .pvReserved = {},
        .dwReserved = {},
        .FlagsEx = {},
    };

    if (GetSaveFileName(&arg) == TRUE)
    {
        return { true, std::filesystem::path(szFile.get()) };
    }
    return { false, std::filesystem::path() };
}
