#pragma once

#include "animation/Frame.hpp"
#include "third_party/nlohmann/json.hpp"
#include <string>
#include <vector>

namespace pixelforge::animation {

struct FrameDiffResult {
    size_t frame_a_index = 0;
    size_t frame_b_index = 0;
    int pixel_difference_count = 0;
    std::vector<std::string> changed_regions;
    std::vector<std::string> unchanged_regions;
    core::Rect bbox_a;
    core::Rect bbox_b;
    int bbox_dx = 0;
    int bbox_dy = 0;
    bool palette_identical = true;

    std::string to_text_format() const;
    nlohmann::json to_json() const;
};

class FrameDiffer {
public:
    static FrameDiffResult compare(const Frame& a, const Frame& b, size_t idx_a = 0, size_t idx_b = 1);
};

} // namespace pixelforge::animation
