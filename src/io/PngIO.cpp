#include "io/PngIO.hpp"
#include "core/Error.hpp"
#include "core/Logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb/stb_image_write.h"

#include <vector>
#include <filesystem>

namespace pixelforge::io {

bool PngIO::save(const sprite::Sprite& sprite, const std::string& filepath) {
    int w = sprite.width();
    int h = sprite.height();
    std::vector<uint8_t> raw_rgba(static_cast<size_t>(w * h * 4));

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            auto c = sprite.get_composite_pixel(x, y);
            size_t idx = static_cast<size_t>((y * w + x) * 4);
            raw_rgba[idx + 0] = c.r;
            raw_rgba[idx + 1] = c.g;
            raw_rgba[idx + 2] = c.b;
            raw_rgba[idx + 3] = c.a;
        }
    }

    // Ensure parent directory exists
    std::filesystem::path p(filepath);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    int stride = w * 4;
    int res = stbi_write_png(filepath.c_str(), w, h, 4, raw_rgba.data(), stride);
    return res != 0;
}

sprite::Sprite PngIO::load(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        throw core::PixelForgeException(core::ErrorCode::IOError, "File does not exist: " + filepath);
    }

    int w = 0, h = 0, channels = 0;
    stbi_uc* data = stbi_load(filepath.c_str(), &w, &h, &channels, 4);
    if (!data) {
        throw core::PixelForgeException(core::ErrorCode::IOError, "Failed to decode PNG: " + filepath);
    }

    std::filesystem::path p(filepath);
    std::string name = p.stem().string();
    sprite::Sprite spr(w, h, name);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            size_t idx = static_cast<size_t>((y * w + x) * 4);
            core::Color c{
                data[idx + 0],
                data[idx + 1],
                data[idx + 2],
                data[idx + 3]
            };
            if (!c.is_transparent()) {
                spr.set_pixel(x, y, c);
            }
        }
    }

    stbi_image_free(data);
    return spr;
}

} // namespace pixelforge::io
