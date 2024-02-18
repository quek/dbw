#pragma once
#include <format>
#include <string>
#include "ErrorWindow.h"

//template<typename... Args>
//void Error(const std::string& format, Args&&... args) {
//    std::string message;
//    if constexpr (sizeof...(args) == 0) {
//        message = format;
//    } else {
//        message = std::format(format, std::forward<Args>(args)...);
//    }
//    gErrorWindow->show(message);
//}

template<typename... Args>
void Error(const std::string& format, Args&&... args) {
    std::string message;
    if constexpr (sizeof...(args) == 0) {
        message = format;
    } else {
        message = std::format(format, std::forward<Args>(args)...);
    }
    gErrorWindow->show(message);
}

//template<typename... Args>
//void Error(std::string& format, Args&&... args) {
//    std::string message;
//    if constexpr (sizeof...(args) == 0) {
//        message = format;
//    } else {
//        message = std::format(format, std::forward<Args>(args)...);
//    }
//    gErrorWindow->show(message);
//}
