#pragma once

#include "animation/Frame.hpp"
#include <vector>
#include <string>
#include <memory>

namespace pixelforge::animation {

class Animation {
public:
    Animation(int width, int height, std::string name = "anim");

    int width() const { return width_; }
    int height() const { return height_; }
    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    size_t frame_count() const { return frames_.size(); }
    bool empty() const { return frames_.empty(); }

    Frame& frame(size_t index);
    const Frame& frame(size_t index) const;

    size_t current_frame_index() const { return current_frame_index_; }
    void set_current_frame_index(size_t index);
    Frame& current_frame();
    const Frame& current_frame() const;

    size_t add_frame(sprite::Sprite sprite, int duration_ms = 100, std::string name = "");
    size_t duplicate_frame(size_t index);
    void insert_frame(size_t index, sprite::Sprite sprite, int duration_ms = 100);
    bool delete_frame(size_t index);
    bool reorder_frame(size_t from_index, size_t to_index);
    bool set_frame_duration(size_t index, int duration_ms);

    void interpolate_region(const std::string& region_name, size_t from_frame, size_t to_frame);
    void copy_region_between_frames(const std::string& region_name, size_t from_frame, size_t to_frame);

    int total_duration_ms() const;
    const std::vector<Frame>& frames() const { return frames_; }

private:
    int width_;
    int height_;
    std::string name_;
    std::vector<Frame> frames_;
    size_t current_frame_index_ = 0;
};

} // namespace pixelforge::animation
