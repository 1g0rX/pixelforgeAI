#include "sprite/Region.hpp"
#include <algorithm>
#include <limits>

namespace pixelforge::sprite {

Region::Region(std::string name) : name_(std::move(name)) {}

Region::Region(std::string name, const core::Rect& rect)
    : name_(std::move(name)), bounds_(rect) {
    set_from_rect(rect);
}

Region::Region(std::string name, std::vector<core::Point> points)
    : name_(std::move(name)), points_(std::move(points)) {
    for (const auto& p : points_) {
        point_set_.insert(p);
    }
    recalculate_bounds();
}

bool Region::contains(int x, int y) const {
    if (!bounds_.contains(x, y)) {
        return false;
    }
    return point_set_.contains(core::Point{x, y});
}

void Region::add_point(int x, int y) {
    core::Point pt{x, y};
    if (point_set_.insert(pt).second) {
        points_.push_back(pt);
        if (points_.size() == 1) {
            bounds_ = {x, y, 1, 1};
        } else {
            int min_x = std::min(bounds_.x, x);
            int min_y = std::min(bounds_.y, y);
            int max_x = std::max(bounds_.right(), x);
            int max_y = std::max(bounds_.bottom(), y);
            bounds_ = {min_x, min_y, max_x - min_x + 1, max_y - min_y + 1};
        }
    }
}

bool Region::remove_point(int x, int y) {
    core::Point pt{x, y};
    if (point_set_.erase(pt) > 0) {
        auto it = std::find(points_.begin(), points_.end(), pt);
        if (it != points_.end()) {
            points_.erase(it);
        }
        recalculate_bounds();
        return true;
    }
    return false;
}

void Region::clear() {
    points_.clear();
    point_set_.clear();
    bounds_ = {0, 0, 0, 0};
}

void Region::set_from_rect(const core::Rect& rect) {
    clear();
    bounds_ = rect;
    if (rect.is_empty()) return;

    points_.reserve(static_cast<size_t>(rect.width * rect.height));
    for (int y = rect.y; y <= rect.bottom(); ++y) {
        for (int x = rect.x; x <= rect.right(); ++x) {
            core::Point pt{x, y};
            points_.push_back(pt);
            point_set_.insert(pt);
        }
    }
}

void Region::recalculate_bounds() {
    if (points_.empty()) {
        bounds_ = {0, 0, 0, 0};
        return;
    }

    int min_x = std::numeric_limits<int>::max();
    int min_y = std::numeric_limits<int>::max();
    int max_x = std::numeric_limits<int>::lowest();
    int max_y = std::numeric_limits<int>::lowest();

    for (const auto& p : points_) {
        min_x = std::min(min_x, p.x);
        min_y = std::min(min_y, p.y);
        max_x = std::max(max_x, p.x);
        max_y = std::max(max_y, p.y);
    }

    bounds_ = {min_x, min_y, max_x - min_x + 1, max_y - min_y + 1};
}

void Region::translate(int dx, int dy) {
    if (dx == 0 && dy == 0) return;
    point_set_.clear();
    for (auto& p : points_) {
        p.x += dx;
        p.y += dy;
        point_set_.insert(p);
    }
    bounds_.x += dx;
    bounds_.y += dy;
}

void Region::mirror_horizontal(int axis_x) {
    point_set_.clear();
    for (auto& p : points_) {
        p.x = 2 * axis_x - p.x;
        point_set_.insert(p);
    }
    recalculate_bounds();
}

void Region::mirror_vertical(int axis_y) {
    point_set_.clear();
    for (auto& p : points_) {
        p.y = 2 * axis_y - p.y;
        point_set_.insert(p);
    }
    recalculate_bounds();
}

} // namespace pixelforge::sprite
