#pragma once
#include <format>
#include <string>
#include "ErrorWindow.h"

template<typename... Args>
void Error(std::format_string<Args...> format, Args&&... args) {
    gErrorWindow->show(std::format(format, std::forward<Args>(args)...));
}

