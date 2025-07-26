#pragma once
#include <string>

namespace Screenshot {
    bool SaveScreenshot(const std::string& filename, int width, int height);
    std::string GenerateScreenshotFilename();
}
