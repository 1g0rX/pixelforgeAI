#include "animation/SpriteSheet.hpp"
#include "core/Error.hpp"
#include <cmath>

namespace pixelforge::animation {

SpriteSheetResult SpriteSheet::generate(const Animation& anim, const SpriteSheetOptions& options) {
    if (anim.empty()) {
        throw core::PixelForgeException(core::ErrorCode::InvalidArgument, "Cannot generate sprite sheet from empty animation");
    }

    std::vector<size_t> indices = options.frame_indices;
    if (indices.empty()) {
        indices.reserve(anim.frame_count());
        for (size_t i = 0; i < anim.frame_count(); ++i) {
            indices.push_back(i);
        }
    }

    int total_frames = static_cast<int>(indices.size());
    int cols = options.columns;
    if (cols <= 0) {
        cols = total_frames; // 1 row by default
    }
    int rows = options.rows;
    if (rows <= 0) {
        rows = static_cast<int>(std::ceil(static_cast<float>(total_frames) / static_cast<float>(cols)));
    }

    int fw = anim.width();
    int fh = anim.height();

    int sheet_w = options.margin * 2 + cols * fw + std::max(0, cols - 1) * options.spacing;
    int sheet_h = options.margin * 2 + rows * fh + std::max(0, rows - 1) * options.spacing;

    sprite::Sprite sheet(sheet_w, sheet_h, anim.name() + "_sheet", core::Transparent);
    nlohmann::json meta;
    meta["animation_name"] = anim.name();
    meta["frame_width"] = fw;
    meta["frame_height"] = fh;
    meta["columns"] = cols;
    meta["rows"] = rows;
    meta["frames"] = nlohmann::json::array();

    for (int i = 0; i < total_frames; ++i) {
        size_t f_idx = indices[i];
        if (f_idx >= anim.frame_count()) continue;

        int c = i % cols;
        int r = i / cols;

        int dest_x = options.margin + c * (fw + options.spacing);
        int dest_y = options.margin + r * (fh + options.spacing);

        const auto& f = anim.frame(f_idx);
        for (int y = 0; y < fh; ++y) {
            for (int x = 0; x < fw; ++x) {
                auto color = f.sprite.get_pixel(x, y);
                if (!color.is_transparent()) {
                    sheet.set_pixel(dest_x + x, dest_y + y, color);
                }
            }
        }

        nlohmann::json frame_info;
        frame_info["index"] = i;
        frame_info["source_frame"] = f_idx;
        frame_info["duration_ms"] = f.duration_ms;
        frame_info["rect"] = {
            {"x", dest_x}, {"y", dest_y}, {"width", fw}, {"height", fh}
        };
        meta["frames"].push_back(frame_info);
    }

    return SpriteSheetResult{
        .sheet_sprite = std::move(sheet),
        .metadata = std::move(meta)
    };
}

} // namespace pixelforge::animation
