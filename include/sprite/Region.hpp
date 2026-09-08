#pragma once

#include "core/Types.hpp"
#include <string>
#include <vector>
#include <unordered_set>

namespace pixelforge::sprite {

struct PointHash {
    size_t operator()(const core::Point& p) const noexcept {
        return (static_cast<size_t>(p.x) << 32) ^ (static_cast<size_t>(p.y) & 0xFFFFFFFF);
    }
};

class Region {
public:
    Region() = default;
    explicit Region(std::string name);
    Region(std::string name, const core::Rect& rect);
    Region(std::string name, std::vector<core::Point> points);

    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    const core::Rect& bounds() const { return bounds_; }
    const std::vector<core::Point>& points() const { return points_; }
    size_t pixel_count() const { return points_.size(); }
    bool empty() const { return points_.empty(); }

    bool contains(int x, int y) const;
    bool contains(const core::Point& pt) const { return contains(pt.x, pt.y); }

    void add_point(int x, int y);
    void add_point(const core::Point& pt) { add_point(pt.x, pt.y); }
    bool remove_point(int x, int y);
    void clear();

    void set_from_rect(const core::Rect& rect);
    void recalculate_bounds();

    void translate(int dx, int dy);
    void mirror_horizontal(int axis_x);
    void mirror_vertical(int axis_y);

private:
    std::string name_;
    core::Rect bounds_{0, 0, 0, 0};
    std::vector<core::Point> points_;
    std::unordered_set<core::Point, PointHash> point_set_;
};

} // namespace pixelforge::sprite
