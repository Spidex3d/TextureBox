#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct Image
{
    int width = 0, height = 0;
    std::vector<uint8_t> pixels; // always RGBA, width*height*4

    bool Load(const std::string& path);
    bool SavePNG(const std::string& path) const;
    bool IsValid() const { return width > 0 && height > 0 && !pixels.empty(); }
};
