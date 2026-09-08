#pragma once

#include "sprite/Sprite.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include "third_party/nlohmann/json.hpp"

namespace pixelforge::sprite {

struct SpriteStateSummary {
    int width = 0;
    int height = 0;
    bool has_transparency = true;
    size_t color_count = 0;
    core::Rect bounding_box;
    std::unordered_map<std::string, core::Rect> regions;
    std::vector<std::string> palette_hex;
    std::string ascii_art;
    std::string ascii_legend;

    std::string to_text_format(bool include_ascii = true) const;
    nlohmann::json to_json() const;
};

class SpriteState {
public:
    static SpriteStateSummary summarize(const Sprite& sprite, bool generate_ascii = true, int max_ascii_dim = 32);
    static std::string generate_ascii_grid(const Sprite& sprite, std::string* out_legend = nullptr);
    static std::string generate_region_ascii(const Sprite& sprite, const std::string& region_name);
};

} // namespace pixelforge::sprite
