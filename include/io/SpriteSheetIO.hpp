#pragma once

#include "animation/Animation.hpp"
#include "animation/SpriteSheet.hpp"
#include <string>

namespace pixelforge::io {

class SpriteSheetIO {
public:
    static bool save(const animation::Animation& anim,
                     const std::string& png_filepath,
                     const animation::SpriteSheetOptions& options = {},
                     const std::string& json_metadata_path = "");
};

} // namespace pixelforge::io
