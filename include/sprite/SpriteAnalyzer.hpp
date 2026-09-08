#pragma once

#include "core/Types.hpp"
#include "sprite/Sprite.hpp"
#include <vector>
#include <string>

namespace pixelforge::sprite {

struct AnalysisOptions {
    size_t max_allowed_colors = 32;
    bool allow_semi_transparency = false;
    int noise_threshold = 1;
};

class SpriteAnalyzer {
public:
    static core::ValidationReport analyze(const Sprite& sprite, const AnalysisOptions& options = {});
    static int count_isolated_pixels(const Sprite& sprite);
    static int count_disconnected_regions(const Sprite& sprite);
    static bool check_anti_aliasing(const Sprite& sprite);
};

} // namespace pixelforge::sprite
