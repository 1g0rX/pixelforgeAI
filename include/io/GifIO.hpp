#pragma once

#include "animation/Animation.hpp"
#include <string>

namespace pixelforge::io {

class GifIO {
public:
    static bool save(const animation::Animation& anim, const std::string& filepath, int loop_count = 0);
};

} // namespace pixelforge::io
