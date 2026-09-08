#pragma once

#include "animation/Animation.hpp"
#include "third_party/nlohmann/json.hpp"
#include <optional>

namespace pixelforge::animation {

struct SpriteSheetOptions {
    int columns = 0;             // 0 = single horizontal row
    int rows = 0;                // 0 = computed automatically
    int spacing = 0;             // Spacing between frames
    int margin = 0;              // Outer margin
    std::vector<size_t> frame_indices; // Custom ordering, empty = all in sequence
};

struct SpriteSheetResult {
    sprite::Sprite sheet_sprite;
    nlohmann::json metadata;
};

class SpriteSheet {
public:
    static SpriteSheetResult generate(const Animation& anim, const SpriteSheetOptions& options = {});
};

} // namespace pixelforge::animation
