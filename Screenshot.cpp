#include "Screenshot.h"
#include <glad/glad.h>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Screenshot {

    std::string GenerateScreenshotFilename() {
        // Create screenshots directory if it doesn't exist
        std::filesystem::create_directories("screenshots");

        // Generate timestamp-based filename
        auto now = std::time(nullptr);

#ifdef _WIN32
        // Use localtime_s for Windows 
        struct tm tm_buf;
        localtime_s(&tm_buf, &now);
        auto tm = tm_buf;
#else
        // Use localtime_r for Unix-like systems 
        struct tm tm_buf;
        localtime_r(&now, &tm_buf);
        auto tm = tm_buf;
#endif

        std::ostringstream oss;
        oss << "screenshots/screenshot_"
            << std::put_time(&tm, "%Y%m%d_%H%M%S")
            << ".png";

        return oss.str();
    }

    bool SaveScreenshot(const std::string& filename, int width, int height) {
        // Allocate memory for pixel data
        unsigned char* pixels = new unsigned char[width * height * 3];

        if (!pixels) {
            std::cerr << "Failed to allocate memory for screenshot" << std::endl;
            return false;
        }

        // Read pixels from framebuffer
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error while reading pixels: " << error << std::endl;
            delete[] pixels;
            return false;
        }

        unsigned char* flipped_pixels = new unsigned char[width * height * 3];
        if (!flipped_pixels) {
            std::cerr << "Failed to allocate memory for flipped image" << std::endl;
            delete[] pixels;
            return false;
        }

        for (int y = 0; y < height; y++) {
            memcpy(
                flipped_pixels + (height - 1 - y) * width * 3,
                pixels + y * width * 3,
                width * 3
            );
        }

        int result = stbi_write_png(filename.c_str(), width, height, 3, flipped_pixels, width * 3);

        delete[] pixels;
        delete[] flipped_pixels;

        if (result) {
            std::cout << "Screenshot saved: " << filename << std::endl;
            return true;
        }
        else {
            std::cerr << "Failed to save screenshot: " << filename << std::endl;
            return false;
        }
    }
}