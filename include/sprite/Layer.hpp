#pragma once

#include "core/Types.hpp"
#include <string>
#include <vector>

namespace pixelforge::sprite {

class Layer {
public:
    Layer(std::string name, int width, int height, core::Color fill_color = core::Transparent);

    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    int width() const { return width_; }
    int height() const { return height_; }

    bool is_visible() const { return visible_; }
    void set_visible(bool visible) { visible_ = visible; }

    float opacity() const { return opacity_; }
    void set_opacity(float opacity) { opacity_ = std::clamp(opacity, 0.0f, 1.0f); }

    bool is_valid_coord(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    core::Color get_pixel(int x, int y) const;
    void set_pixel(int x, int y, const core::Color& color);
    void clear_pixel(int x, int y);
    void fill(const core::Color& color);

    const std::vector<core::Color>& pixels() const { return pixels_; }
    std::vector<core::Color>& pixels() { return pixels_; }

    core::Rect calculate_bounding_box() const;
    void resize(int new_width, int new_height, core::Color fill_color = core::Transparent);

private:
    std::string name_;
    int width_;
    int height_;
    std::vector<core::Color> pixels_;
    bool visible_ = true;
    float opacity_ = 1.0f;
};

} // namespace pixelforge::sprite
