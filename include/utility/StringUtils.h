#pragma once
#include <vector>
#include <string>
#include <sstream>

#include <raylib.h>

namespace StringUtils {
    // Splits a long string into multiple lines based on a maximum pixel width
    std::vector<std::string> Wrap(const std::string& txt, int maxPx, int fontSize);
}