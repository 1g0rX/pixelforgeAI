#include "sprite/Palette.hpp"
#include "core/Error.hpp"
#include <limits>
#include <algorithm>

namespace pixelforge::sprite {

Palette::Palette() : Palette(default_palette()) {}

Palette::Palette(std::vector<core::Color> colors) : colors_(std::move(colors)) {}

const core::Color& Palette::get_color(size_t index) const {
    if (index >= colors_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Palette index out of bounds");
    }
    return colors_[index];
}

std::optional<size_t> Palette::find_index(const core::Color& color) const {
    for (size_t i = 0; i < colors_.size(); ++i) {
        if (colors_[i] == color) {
            return i;
        }
    }
    return std::nullopt;
}

bool Palette::has_color(const core::Color& color) const {
    return find_index(color).has_value();
}

size_t Palette::add_color(const core::Color& color) {
    auto existing = find_index(color);
    if (existing.has_value()) {
        return *existing;
    }
    colors_.push_back(color);
    return colors_.size() - 1;
}

bool Palette::remove_color(const core::Color& color) {
    auto it = std::find(colors_.begin(), colors_.end(), color);
    if (it != colors_.end()) {
        colors_.erase(it);
        return true;
    }
    return false;
}

bool Palette::remove_color_at(size_t index) {
    if (index < colors_.size()) {
        colors_.erase(colors_.begin() + index);
        return true;
    }
    return false;
}

void Palette::clear() {
    colors_.clear();
}

void Palette::set_colors(std::vector<core::Color> colors) {
    colors_ = std::move(colors);
}

size_t Palette::find_nearest_index(const core::Color& color, bool include_alpha) const {
    if (colors_.empty()) {
        throw core::PixelForgeException(core::ErrorCode::InvalidArgument, "Palette is empty");
    }
    if (color.is_transparent()) {
        // Find transparent entry if exists
        for (size_t i = 0; i < colors_.size(); ++i) {
            if (colors_[i].is_transparent()) return i;
        }
    }

    size_t best_idx = 0;
    int best_dist = std::numeric_limits<int>::max();

    for (size_t i = 0; i < colors_.size(); ++i) {
        if (!include_alpha && colors_[i].is_transparent() && !color.is_transparent()) {
            continue;
        }
        int dist = color.distance_sq(colors_[i]);
        if (include_alpha) {
            int da = static_cast<int>(color.a) - static_cast<int>(colors_[i].a);
            dist += da * da;
        }
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = i;
        }
    }
    return best_idx;
}

core::Color Palette::find_nearest_color(const core::Color& color, bool include_alpha) const {
    return get_color(find_nearest_index(color, include_alpha));
}

void Palette::quantize(size_t max_colors) {
    if (max_colors == 0) max_colors = 1;
    if (colors_.size() <= max_colors) return;

    // Agglomerative hierarchical clustering / color pair reduction
    while (colors_.size() > max_colors) {
        int min_dist = std::numeric_limits<int>::max();
        size_t merge_i = 0;
        size_t merge_j = 1;

        for (size_t i = 0; i < colors_.size(); ++i) {
            // Keep transparent intact if present
            if (colors_[i].is_transparent()) continue;
            for (size_t j = i + 1; j < colors_.size(); ++j) {
                if (colors_[j].is_transparent()) continue;
                int dist = colors_[i].distance_sq(colors_[j]);
                if (dist < min_dist) {
                    min_dist = dist;
                    merge_i = i;
                    merge_j = j;
                }
            }
        }

        if (min_dist == std::numeric_limits<int>::max()) break;

        // Average colors
        const auto& c1 = colors_[merge_i];
        const auto& c2 = colors_[merge_j];
        core::Color avg{
            static_cast<uint8_t>((static_cast<int>(c1.r) + c2.r) / 2),
            static_cast<uint8_t>((static_cast<int>(c1.g) + c2.g) / 2),
            static_cast<uint8_t>((static_cast<int>(c1.b) + c2.b) / 2),
            static_cast<uint8_t>((static_cast<int>(c1.a) + c2.a) / 2)
        };

        colors_[merge_i] = avg;
        colors_.erase(colors_.begin() + merge_j);
    }
}

Palette Palette::default_palette() {
    return Palette({
        core::Color{0, 0, 0, 0},         // Transparent
        core::Color{0, 0, 0, 255},       // Black
        core::Color{255, 255, 255, 255}, // White
        core::Color{128, 128, 128, 255}, // Gray
        core::Color{192, 192, 192, 255}, // Light Gray
        core::Color{136, 0, 0, 255},     // Dark Red
        core::Color{255, 0, 0, 255},     // Red
        core::Color{255, 128, 0, 255},   // Orange
        core::Color{255, 255, 0, 255},   // Yellow
        core::Color{0, 136, 0, 255},     // Dark Green
        core::Color{0, 255, 0, 255},     // Green
        core::Color{0, 136, 136, 255},   // Dark Cyan
        core::Color{0, 255, 255, 255},   // Cyan
        core::Color{0, 0, 136, 255},     // Dark Blue
        core::Color{0, 0, 255, 255},     // Blue
        core::Color{136, 0, 136, 255},   // Purple
        core::Color{255, 0, 255, 255},   // Magenta
        core::Color{136, 68, 0, 255},    // Brown
    });
}

Palette Palette::pico8() {
    return Palette({
        core::Color{0, 0, 0, 0},            // Transparent
        core::Color::from_hex("#000000"),   // 0: Black
        core::Color::from_hex("#1D2B53"),   // 1: Dark Blue
        core::Color::from_hex("#7E2553"),   // 2: Dark Purple
        core::Color::from_hex("#008751"),   // 3: Dark Green
        core::Color::from_hex("#AB5236"),   // 4: Brown
        core::Color::from_hex("#5F574F"),   // 5: Dark Gray
        core::Color::from_hex("#C2C3C7"),   // 6: Light Gray
        core::Color::from_hex("#FFF1E8"),   // 7: White
        core::Color::from_hex("#FF004D"),   // 8: Red
        core::Color::from_hex("#FFA300"),   // 9: Orange
        core::Color::from_hex("#FFEC27"),   // 10: Yellow
        core::Color::from_hex("#00E436"),   // 11: Green
        core::Color::from_hex("#29ADFF"),   // 12: Blue
        core::Color::from_hex("#83769C"),   // 13: Indigo
        core::Color::from_hex("#FF77A8"),   // 14: Pink
        core::Color::from_hex("#FFCCAA")    // 15: Peach
    });
}

Palette Palette::gameboy() {
    return Palette({
        core::Color{0, 0, 0, 0},            // Transparent
        core::Color::from_hex("#0F380F"),   // Darkest
        core::Color::from_hex("#306230"),   // Dark
        core::Color::from_hex("#8BAC0F"),   // Light
        core::Color::from_hex("#9BBC0F")    // Lightest
    });
}

Palette Palette::dawnbringer16() {
    return Palette({
        core::Color{0, 0, 0, 0},            // Transparent
        core::Color::from_hex("#140C1C"),
        core::Color::from_hex("#442434"),
        core::Color::from_hex("#30346D"),
        core::Color::from_hex("#4E4A4E"),
        core::Color::from_hex("#854C30"),
        core::Color::from_hex("#346524"),
        core::Color::from_hex("#D04648"),
        core::Color::from_hex("#757161"),
        core::Color::from_hex("#597DCE"),
        core::Color::from_hex("#D27D2C"),
        core::Color::from_hex("#8595A1"),
        core::Color::from_hex("#6DAA2C"),
        core::Color::from_hex("#D2AA99"),
        core::Color::from_hex("#6DC2CA"),
        core::Color::from_hex("#DAD45E"),
        core::Color::from_hex("#DEEED6")
    });
}

Palette Palette::dawnbringer32() {
    return Palette({
        core::Color{0, 0, 0, 0},
        core::Color::from_hex("#000000"), core::Color::from_hex("#222034"),
        core::Color::from_hex("#45283C"), core::Color::from_hex("#663931"),
        core::Color::from_hex("#8F563B"), core::Color::from_hex("#DF7126"),
        core::Color::from_hex("#D9A066"), core::Color::from_hex("#EEC39A"),
        core::Color::from_hex("#FBF236"), core::Color::from_hex("#99E550"),
        core::Color::from_hex("#6ABE30"), core::Color::from_hex("#37946E"),
        core::Color::from_hex("#4B692F"), core::Color::from_hex("#524B24"),
        core::Color::from_hex("#323C39"), core::Color::from_hex("#3F3F74"),
        core::Color::from_hex("#306082"), core::Color::from_hex("#5B6EE1"),
        core::Color::from_hex("#639BFF"), core::Color::from_hex("#5FCDE4"),
        core::Color::from_hex("#CBDBFC"), core::Color::from_hex("#FFFFFF"),
        core::Color::from_hex("#9BADB7"), core::Color::from_hex("#847E87"),
        core::Color::from_hex("#696A6A"), core::Color::from_hex("#595652"),
        core::Color::from_hex("#76428A"), core::Color::from_hex("#AC3232"),
        core::Color::from_hex("#D95763"), core::Color::from_hex("#D77BBA"),
        core::Color::from_hex("#8F974A"), core::Color::from_hex("#8A6F30")
    });
}

Palette Palette::endesga32() {
    return Palette({
        core::Color{0, 0, 0, 0},
        core::Color::from_hex("#BE4A2F"), core::Color::from_hex("#D77643"),
        core::Color::from_hex("#EAD4AA"), core::Color::from_hex("#E4A672"),
        core::Color::from_hex("#B86F50"), core::Color::from_hex("#733E39"),
        core::Color::from_hex("#3E2731"), core::Color::from_hex("#A22633"),
        core::Color::from_hex("#E43B44"), core::Color::from_hex("#F77622"),
        core::Color::from_hex("#FEE761"), core::Color::from_hex("#63C74D"),
        core::Color::from_hex("#3E8948"), core::Color::from_hex("#265C42"),
        core::Color::from_hex("#193C3E"), core::Color::from_hex("#124E89"),
        core::Color::from_hex("#0099DB"), core::Color::from_hex("#2CE8F5"),
        core::Color::from_hex("#FFFFFF"), core::Color::from_hex("#C0CBDC"),
        core::Color::from_hex("#8B9BB4"), core::Color::from_hex("#5A6988"),
        core::Color::from_hex("#3A4466"), core::Color::from_hex("#262B44"),
        core::Color::from_hex("#181425"), core::Color::from_hex("#FF0044"),
        core::Color::from_hex("#68386C"), core::Color::from_hex("#B55088"),
        core::Color::from_hex("#F6757A"), core::Color::from_hex("#E8B796"),
        core::Color::from_hex("#C28569")
    });
}

Palette Palette::grayscale(size_t shades) {
    if (shades < 2) shades = 2;
    std::vector<core::Color> colors;
    colors.reserve(shades + 1);
    colors.push_back(core::Color{0, 0, 0, 0}); // Transparent
    for (size_t i = 0; i < shades; ++i) {
        uint8_t val = static_cast<uint8_t>(std::round((255.0 * i) / (shades - 1)));
        colors.push_back(core::Color{val, val, val, 255});
    }
    return Palette(std::move(colors));
}

} // namespace pixelforge::sprite
