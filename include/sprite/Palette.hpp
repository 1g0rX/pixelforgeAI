#pragma once

#include "core/Types.hpp"
#include <vector>
#include <string>
#include <optional>
#include <span>

namespace pixelforge::sprite {

class Palette {
public:
    Palette();
    explicit Palette(std::vector<core::Color> colors);

    size_t size() const { return colors_.size(); }
    bool empty() const { return colors_.empty(); }

    const std::vector<core::Color>& colors() const { return colors_; }
    const core::Color& get_color(size_t index) const;

    std::optional<size_t> find_index(const core::Color& color) const;
    bool has_color(const core::Color& color) const;

    size_t add_color(const core::Color& color);
    bool remove_color(const core::Color& color);
    bool remove_color_at(size_t index);
    void clear();

    void set_colors(std::vector<core::Color> colors);

    size_t find_nearest_index(const core::Color& color, bool include_alpha = false) const;
    core::Color find_nearest_color(const core::Color& color, bool include_alpha = false) const;

    void quantize(size_t max_colors);

    // Preset palettes
    static Palette default_palette();
    static Palette pico8();
    static Palette gameboy();
    static Palette dawnbringer16();
    static Palette dawnbringer32();
    static Palette endesga32();
    static Palette grayscale(size_t shades = 8);

private:
    std::vector<core::Color> colors_;
};

} // namespace pixelforge::sprite
