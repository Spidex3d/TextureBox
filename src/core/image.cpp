#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <core/Image.h>

bool Image::Load(const std::string& path)
{
    int c;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &c, 4);
    if (!data) return false;
    pixels.assign(data, data + width * height * 4);
    stbi_image_free(data);
    return true;
}

bool Image::SavePNG(const std::string& path) const
{
    return IsValid() && stbi_write_png(path.c_str(), width, height, 4, pixels.data(), width * 4);
}