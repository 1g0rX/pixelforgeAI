#include "animation/Animation.hpp"
#include "core/Error.hpp"
#include <cmath>
#include <algorithm>

namespace pixelforge::animation {

Animation::Animation(int width, int height, std::string name)
    : width_(width), height_(height), name_(std::move(name)) {
    if (width_ <= 0 || height_ <= 0) {
        throw core::PixelForgeException(core::ErrorCode::InvalidArgument, "Animation dimensions must be positive");
    }
}

Frame& Animation::frame(size_t index) {
    if (index >= frames_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Frame index out of bounds");
    }
    return frames_[index];
}

const Frame& Animation::frame(size_t index) const {
    if (index >= frames_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Frame index out of bounds");
    }
    return frames_[index];
}

void Animation::set_current_frame_index(size_t index) {
    if (index >= frames_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Frame index out of bounds");
    }
    current_frame_index_ = index;
}

Frame& Animation::current_frame() {
    return frame(current_frame_index_);
}

const Frame& Animation::current_frame() const {
    return frame(current_frame_index_);
}

size_t Animation::add_frame(sprite::Sprite sprite, int duration_ms, std::string name) {
    frames_.emplace_back(std::move(sprite), duration_ms, std::move(name));
    return frames_.size() - 1;
}

size_t Animation::duplicate_frame(size_t index) {
    if (index >= frames_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Frame index out of bounds");
    }
    const auto& src = frames_[index];
    frames_.emplace_back(src.sprite, src.duration_ms, src.name + "_copy");
    return frames_.size() - 1;
}

void Animation::insert_frame(size_t index, sprite::Sprite sprite, int duration_ms) {
    if (index > frames_.size()) {
        index = frames_.size();
    }
    frames_.emplace(frames_.begin() + index, std::move(sprite), duration_ms);
}

bool Animation::delete_frame(size_t index) {
    if (frames_.size() <= 1 || index >= frames_.size()) {
        return false;
    }
    frames_.erase(frames_.begin() + index);
    if (current_frame_index_ >= frames_.size()) {
        current_frame_index_ = frames_.size() - 1;
    }
    return true;
}

bool Animation::reorder_frame(size_t from_index, size_t to_index) {
    if (from_index >= frames_.size() || to_index >= frames_.size() || from_index == to_index) {
        return false;
    }
    auto f = std::move(frames_[from_index]);
    frames_.erase(frames_.begin() + from_index);
    frames_.insert(frames_.begin() + to_index, std::move(f));
    return true;
}

bool Animation::set_frame_duration(size_t index, int duration_ms) {
    if (index >= frames_.size() || duration_ms <= 0) {
        return false;
    }
    frames_[index].duration_ms = duration_ms;
    return true;
}

void Animation::interpolate_region(const std::string& region_name, size_t from_frame, size_t to_frame) {
    if (from_frame >= frames_.size() || to_frame >= frames_.size() || from_frame >= to_frame) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Invalid frame range for interpolation");
    }

    const auto& src_sprite = frames_[from_frame].sprite;
    const auto& dst_sprite = frames_[to_frame].sprite;

    if (!src_sprite.has_region(region_name) || !dst_sprite.has_region(region_name)) {
        throw core::PixelForgeException(core::ErrorCode::InvalidRegion, "Region '" + region_name + "' must exist in both keyframes");
    }

    auto b_start = src_sprite.get_region(region_name).bounds();
    auto b_end = dst_sprite.get_region(region_name).bounds();

    int total_dx = b_end.x - b_start.x;
    int total_dy = b_end.y - b_start.y;
    size_t span = to_frame - from_frame;

    for (size_t f = from_frame + 1; f < to_frame; ++f) {
        float t = static_cast<float>(f - from_frame) / static_cast<float>(span);
        int cur_dx = static_cast<int>(std::round(total_dx * t));
        int cur_dy = static_cast<int>(std::round(total_dy * t));

        auto& target_sprite = frames_[f].sprite;
        // Erase old region if exists
        if (target_sprite.has_region(region_name)) {
            target_sprite.erase_region(region_name);
        }

        // Copy from start sprite and move by (cur_dx, cur_dy)
        const auto& start_reg = src_sprite.get_region(region_name);
        std::vector<core::Point> new_points;
        for (const auto& pt : start_reg.points()) {
            auto c = src_sprite.get_pixel(pt.x, pt.y);
            int nx = pt.x + cur_dx;
            int ny = pt.y + cur_dy;
            if (target_sprite.is_valid_coord(nx, ny)) {
                target_sprite.set_pixel(nx, ny, c);
                new_points.push_back({nx, ny});
            }
        }
        target_sprite.create_region(region_name, std::move(new_points));
    }
}

void Animation::copy_region_between_frames(const std::string& region_name, size_t from_frame, size_t to_frame) {
    if (from_frame >= frames_.size() || to_frame >= frames_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Frame index out of bounds");
    }

    const auto& src_sprite = frames_[from_frame].sprite;
    auto& dst_sprite = frames_[to_frame].sprite;

    if (!src_sprite.has_region(region_name)) {
        throw core::PixelForgeException(core::ErrorCode::InvalidRegion, "Region not found in source frame: " + region_name);
    }

    const auto& reg = src_sprite.get_region(region_name);
    std::vector<core::Point> pts;
    for (const auto& pt : reg.points()) {
        auto c = src_sprite.get_pixel(pt.x, pt.y);
        if (dst_sprite.is_valid_coord(pt.x, pt.y)) {
            dst_sprite.set_pixel(pt.x, pt.y, c);
            pts.push_back(pt);
        }
    }
    dst_sprite.create_region(region_name, std::move(pts));
}

int Animation::total_duration_ms() const {
    int sum = 0;
    for (const auto& f : frames_) {
        sum += f.duration_ms;
    }
    return sum;
}

} // namespace pixelforge::animation
