#pragma once

#include "sprite/Sprite.hpp"
#include <string>

namespace pixelforge::io {

class PngIO {
public:
    static bool save(const sprite::Sprite& sprite, const std::string& filepath);
    static sprite::Sprite load(const std::string& filepath);
};

} // namespace pixelforge::io
