#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace path {
    std::string searchExecutable(std::string_view exec);
    std::vector<std::string> listExecutables();
}
