#include "Error.h"
#include "ErrorWindow.h"

void Error(const std::string& message) {
    gErrorWindow->show(message);
}
