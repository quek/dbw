#pragma once
#include <cstdint>
#include <windows.h>

class Module;

class PluginEditorWindow {
public:
    PluginEditorWindow(Module* module, uint32_t width, uint32_t height, bool resizable);
    virtual ~PluginEditorWindow();
    void setSize(uint32_t width, uint32_t height);

    HWND _hwnd = {};
    WNDCLASSEXW _wndClass = {};
private:
    Module* _module;
    bool _resizable;
};
