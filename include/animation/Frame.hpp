#pragma once

#include "sprite/Sprite.hpp"
#include <string>
#include <unordered_map>

namespace pixelforge::animation {

struct Frame {
    sprite::Sprite sprite;
    int duration_ms = 100;
    std::string name;
    std::unordered_map<std::string, std::string> metadata;

    Frame(sprite::Sprite s, int duration = 100, std::string fname = "")
        : sprite(std::move(s)), duration_ms(duration), name(std::move(fname)) {}
};

} // namespace pixelforge::animation
