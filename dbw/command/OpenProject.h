#pragma once
#include <filesystem>
#include "../Command.h"

namespace command {

class OpenProject : public Command {
public:
    OpenProject(const std::filesystem::path& path);
    void execute(Composer* composer) override;
    void undo(Composer*) override {};
private:
    std::filesystem::path _path;
};

};
