#include "sprite/Layer.hpp"
#include "core/Error.hpp"
#include <algorithm>

namespace pixelforge::sprite {

Layer::Layer(std::string name, int width, int height, core::Color fill_color)
    : name_(std::move(name)), width_(width), height_(height),
      pixels_(static_cast<size_t>(width * height), fill_color) {
    if (width_ <= 0 || height_ <= 0) {
        throw core::PixelForgeException(core::ErrorCode::InvalidArgument, "Layer dimensions must be positive");
    }
}

core::Color Layer::get_pixel(int x, int y) const {
    if (!is_valid_coord(x, y)) {
        return core::Transparent;
    }
    return pixels_[static_cast<size_t>(y * width_ + x)];
}

void Layer::set_pixel(int x, int y, const core::Color& color) {
    if (!is_valid_coord(x, y)) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Pixel coordinate out of bounds");
    }
    pixels_[static_cast<size_t>(y * width_ + x)] = color;
}

void Layer::clear_pixel(int x, int y) {
    if (is_valid_coord(x, y)) {
        pixels_[static_cast<size_t>(y * width_ + x)] = core::Transparent;
    }
}

void Layer::fill(const core::Color& color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

core::Rect Layer::calculate_bounding_box() const {
    int min_x = width_;
    int min_y = height_;
    int max_x = -1;
    int max_y = -1;

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            const auto& c = pixels_[static_cast<size_t>(y * width_ + x)];
            if (!c.is_transparent()) {
                min_x = std::min(min_x, x);
                min_y = std::min(min_y, y);
                max_x = std::max(max_x, x);
                max_y = std::max(max_y, y);
            }
        }
    }

    if (max_x == -1) {
        return {0, 0, 0, 0};
    }
    return {min_x, min_y, max_x - min_x + 1, max_y - min_y + 1};
}

void Layer::resize(int new_width, int new_height, core::Color fill_color) {
    if (new_width <= 0 || new_height <= 0) {
        throw core::PixelForgeException(core::ErrorCode::InvalidArgument, "New dimensions must be positive");
    }
    std::vector<core::Color> new_pixels(static_cast<size_t>(new_width * new_height), fill_color);
    int copy_w = std::min(width_, new_width);
    int copy_h = std::min(height_, new_height);

    for (int y = 0; y < copy_h; ++y) {
        for (int x = 0; x < copy_w; ++x) {
            new_pixels[static_cast<size_t>(y * new_width + x)] = pixels_[static_cast<size_t>(y * width_ + x)];
        }
    }

    width_ = new_width;
    height_ = new_height;
    pixels_ = std::move(new_pixels);
}

} // namespace pixelforge::sprite
