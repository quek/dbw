#include "PluginEditorWindow.h"
#include "Module.h"

extern HWND gHwnd;

LRESULT WINAPI Vsit3EditorWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_ERASEBKGND:
    {
        return TRUE; // do not draw background
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return FALSE;
    }
    case WM_SIZE: {
        Module* module = (Module*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (module && wParam != SIZE_MINIMIZED) {
            // TODO なにかする
            RECT r;
            GetClientRect(hWnd, &r);
            module->onResize(r.right - r.left, r.bottom - r.top);
        }
        return 0;
    }
    case WM_DESTROY: {
        Module* module = (Module*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (module) {
            module->closeGui();
        }
        break;
    }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

PluginEditorWindow::PluginEditorWindow(Module* module, int width, int height, bool resizable) : _module(module), _resizable(resizable) {
    RECT rect{ 0, 0, width, height };
    DWORD exStyle = WS_EX_APPWINDOW;
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS;
    if (_resizable)
        dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rect, dwStyle, false, exStyle);

    _wndClass = WNDCLASSEXW{
        .cbSize = sizeof(_wndClass),
        .style = CS_DBLCLKS,
        .lpfnWndProc = Vsit3EditorWndProc,
        .cbClsExtra = 0L,
        .cbWndExtra = 0L,
        .hInstance = GetModuleHandle(nullptr),
        .hIcon = nullptr,
        .hCursor = nullptr,
        .hbrBackground = nullptr,
        .lpszMenuName = nullptr,
        .lpszClassName = L"VST3 Editor",
        .hIconSm = nullptr };
    RegisterClassEx(&_wndClass);
    // gHwnd を設定するとプラグイン編集ウインドが前面に表示され
    _hwnd = CreateWindowEx(exStyle, _wndClass.lpszClassName, L"VST3 Editor", dwStyle,
                           CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                           gHwnd, nullptr, _wndClass.hInstance, nullptr);
    if (!_hwnd) {
        printf("CreateWindosW failed!");
        return;
    }
    SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)module);

    SetWindowPos(_hwnd, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_NOMOVE | SWP_NOCOPYBITS | SWP_SHOWWINDOW);
}

PluginEditorWindow::~PluginEditorWindow() {
    ::DestroyWindow(_hwnd);
    ::UnregisterClassW(_wndClass.lpszClassName, _wndClass.hInstance);
}

void PluginEditorWindow::setSize(uint32_t width, uint32_t height) {
    RECT r;
    GetClientRect(_hwnd, &r);
    if (r.right - r.left == static_cast<LONG>(width) && r.bottom - r.top == static_cast<LONG>(height)) {
        return;
    }
    WINDOWINFO windowInfo{ 0 };
    GetWindowInfo(_hwnd, &windowInfo);
    RECT clientRect{};
    clientRect.right = width;
    clientRect.bottom = height;
    AdjustWindowRectEx(&clientRect, windowInfo.dwStyle, false, windowInfo.dwExStyle);
    SetWindowPos(_hwnd, HWND_TOP, 0, 0, clientRect.right - clientRect.left,
                 clientRect.bottom - clientRect.top, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOACTIVATE);

}
