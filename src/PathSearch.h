#pragma once
#include <string>
#include <string_view>

namespace path {
    std::string searchExecutable(std::string_view exec);
}
