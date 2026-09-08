#include "io/GifIO.hpp"
#include "core/Error.hpp"
#include "core/Logger.hpp"
#include "third_party/gifenc/gifenc.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

namespace pixelforge::io {

bool GifIO::save(const animation::Animation& anim, const std::string& filepath, int loop_count) {
    if (anim.empty()) {
        throw core::PixelForgeException(core::ErrorCode::InvalidArgument, "Cannot save empty animation as GIF");
    }

    // 1. Gather all unique colors across all frames
    std::vector<core::Color> global_palette;
    std::unordered_map<uint32_t, uint8_t> color_to_idx;

    // Index 0 reserved for transparent background
    global_palette.push_back(core::Transparent);
    color_to_idx[core::Transparent.to_u32()] = 0;

    for (const auto& f : anim.frames()) {
        for (int y = 0; y < anim.height(); ++y) {
            for (int x = 0; x < anim.width(); ++x) {
                auto c = f.sprite.get_pixel(x, y);
                if (!c.is_transparent()) {
                    uint32_t u = c.to_u32();
                    if (!color_to_idx.contains(u)) {
                        if (global_palette.size() < 256) {
                            color_to_idx[u] = static_cast<uint8_t>(global_palette.size());
                            global_palette.push_back(c);
                        }
                    }
                }
            }
        }
    }

    // Calculate required depth (1..8)
    int depth = 1;
    while ((1 << depth) < static_cast<int>(global_palette.size()) && depth < 8) {
        depth++;
    }

    size_t palette_capacity = static_cast<size_t>(1 << depth);
    std::vector<uint8_t> pal_bytes(palette_capacity * 3, 0);

    for (size_t i = 0; i < global_palette.size(); ++i) {
        pal_bytes[i * 3 + 0] = global_palette[i].r;
        pal_bytes[i * 3 + 1] = global_palette[i].g;
        pal_bytes[i * 3 + 2] = global_palette[i].b;
    }

    // Ensure parent directories exist
    std::filesystem::path p(filepath);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    ge_GIF* gif = ge_new_gif(
        filepath.c_str(),
        static_cast<uint16_t>(anim.width()),
        static_cast<uint16_t>(anim.height()),
        pal_bytes.data(),
        depth,
        0, // bgindex = 0 (transparent)
        loop_count
    );

    if (!gif) {
        return false;
    }

    int w = anim.width();
    int h = anim.height();

    for (const auto& f : anim.frames()) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                auto c = f.sprite.get_pixel(x, y);
                uint8_t idx = 0;
                if (!c.is_transparent()) {
                    uint32_t u = c.to_u32();
                    auto it = color_to_idx.find(u);
                    if (it != color_to_idx.end()) {
                        idx = it->second;
                    } else {
                        // Nearest match fallback if palette overflowed 256 colors
                        int best_dist = 9999999;
                        for (size_t pi = 1; pi < global_palette.size(); ++pi) {
                            int d = c.distance_sq(global_palette[pi]);
                            if (d < best_dist) {
                                best_dist = d;
                                idx = static_cast<uint8_t>(pi);
                            }
                        }
                    }
                }
                gif->frame[y * w + x] = idx;
            }
        }
        // Delay in hundredths of a second (10ms units)
        uint16_t delay = static_cast<uint16_t>(std::max(1, f.duration_ms / 10));
        ge_add_frame(gif, delay);
    }

    ge_close_gif(gif);
    return true;
}

} // namespace pixelforge::io
