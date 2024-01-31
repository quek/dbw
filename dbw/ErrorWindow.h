#pragma once
#include <string>

class ErrorWindow {
public:
    void render();
    void show(const std::string message);
    void show(const std::string message, const std::exception& e);
private:
    std::string _message = "";
    bool _show = false;
};

extern ErrorWindow* gErrorWindow;
