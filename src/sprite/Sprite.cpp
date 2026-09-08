#include "sprite/Sprite.hpp"
#include "core/Error.hpp"
#include <queue>
#include <cmath>
#include <algorithm>
#include <unordered_set>

namespace pixelforge::sprite {

Sprite::Sprite() : Sprite(32, 32, "untitled", core::Transparent) {}

Sprite::Sprite(int width, int height, std::string name, core::Color background_color)
    : width_(width), height_(height), name_(std::move(name)),
      palette_(Palette::default_palette()) {
    if (width_ <= 0 || height_ <= 0) {
        throw core::PixelForgeException(core::ErrorCode::InvalidArgument, "Sprite dimensions must be positive");
    }
    layers_.emplace_back("Base", width_, height_, background_color);
}

Layer& Sprite::layer(size_t index) {
    if (index >= layers_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Layer index out of bounds");
    }
    return layers_[index];
}

const Layer& Sprite::layer(size_t index) const {
    if (index >= layers_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Layer index out of bounds");
    }
    return layers_[index];
}

Layer& Sprite::active_layer() {
    return layer(active_layer_index_);
}

const Layer& Sprite::active_layer() const {
    return layer(active_layer_index_);
}

void Sprite::set_active_layer(size_t index) {
    if (index >= layers_.size()) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Layer index out of bounds");
    }
    active_layer_index_ = index;
}

size_t Sprite::add_layer(std::string name, core::Color fill_color) {
    layers_.emplace_back(std::move(name), width_, height_, fill_color);
    return layers_.size() - 1;
}

bool Sprite::remove_layer(size_t index) {
    if (layers_.size() <= 1 || index >= layers_.size()) {
        return false;
    }
    layers_.erase(layers_.begin() + index);
    if (active_layer_index_ >= layers_.size()) {
        active_layer_index_ = layers_.size() - 1;
    }
    return true;
}

const Region& Sprite::get_region(const std::string& name) const {
    auto it = regions_.find(name);
    if (it == regions_.end()) {
        throw core::PixelForgeException(core::ErrorCode::InvalidRegion, "Region not found: " + name);
    }
    return it->second;
}

Region& Sprite::get_region(const std::string& name) {
    auto it = regions_.find(name);
    if (it == regions_.end()) {
        throw core::PixelForgeException(core::ErrorCode::InvalidRegion, "Region not found: " + name);
    }
    return it->second;
}

void Sprite::create_region(const std::string& name, const core::Rect& rect) {
    regions_[name] = Region(name, rect);
}

void Sprite::create_region(const std::string& name, std::vector<core::Point> points) {
    regions_[name] = Region(name, std::move(points));
}

bool Sprite::rename_region(const std::string& old_name, const std::string& new_name) {
    auto it = regions_.find(old_name);
    if (it == regions_.end() || regions_.contains(new_name)) {
        return false;
    }
    Region r = std::move(it->second);
    r.set_name(new_name);
    regions_.erase(it);
    regions_.emplace(new_name, std::move(r));
    return true;
}

bool Sprite::delete_region(const std::string& name) {
    return regions_.erase(name) > 0;
}

core::Color Sprite::get_pixel(int x, int y) const {
    return active_layer().get_pixel(x, y);
}

void Sprite::set_pixel(int x, int y, const core::Color& color) {
    if (!is_valid_coord(x, y)) {
        throw core::PixelForgeException(core::ErrorCode::OutOfBounds, "Pixel coordinate out of bounds");
    }
    active_layer().set_pixel(x, y, color);
    if (!color.is_transparent()) {
        palette_.add_color(color);
    }
}

void Sprite::clear_pixel(int x, int y) {
    if (is_valid_coord(x, y)) {
        active_layer().clear_pixel(x, y);
    }
}

core::Color Sprite::get_composite_pixel(int x, int y) const {
    if (!is_valid_coord(x, y)) {
        return core::Transparent;
    }
    core::Color comp = core::Transparent;
    for (const auto& l : layers_) {
        if (!l.is_visible()) continue;
        core::Color c = l.get_pixel(x, y);
        if (c.a > 0) {
            comp = c.blend_over(comp);
        }
    }
    return comp;
}

std::vector<core::Color> Sprite::get_composite_pixels() const {
    std::vector<core::Color> result(static_cast<size_t>(width_ * height_), core::Transparent);
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            result[static_cast<size_t>(y * width_ + x)] = get_composite_pixel(x, y);
        }
    }
    return result;
}

void Sprite::draw_line(int x0, int y0, int x1, int y1, const core::Color& color) {
    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        if (is_valid_coord(x0, y0)) {
            set_pixel(x0, y0, color);
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Sprite::draw_rectangle(int x, int y, int w, int h, const core::Color& color, bool fill) {
    if (w <= 0 || h <= 0) return;
    if (fill) {
        for (int py = y; py < y + h; ++py) {
            for (int px = x; px < x + w; ++px) {
                if (is_valid_coord(px, py)) {
                    set_pixel(px, py, color);
                }
            }
        }
    } else {
        // Outline
        for (int px = x; px < x + w; ++px) {
            if (is_valid_coord(px, y)) set_pixel(px, y, color);
            if (is_valid_coord(px, y + h - 1)) set_pixel(px, y + h - 1, color);
        }
        for (int py = y + 1; py < y + h - 1; ++py) {
            if (is_valid_coord(x, py)) set_pixel(x, py, color);
            if (is_valid_coord(x + w - 1, py)) set_pixel(x + w - 1, py, color);
        }
    }
}

void Sprite::draw_circle(int cx, int cy, int radius, const core::Color& color, bool fill) {
    if (radius < 0) return;
    if (radius == 0) {
        if (is_valid_coord(cx, cy)) set_pixel(cx, cy, color);
        return;
    }

    if (fill) {
        for (int y = -radius; y <= radius; ++y) {
            int dx = static_cast<int>(std::round(std::sqrt(radius * radius - y * y)));
            for (int x = -dx; x <= dx; ++x) {
                if (is_valid_coord(cx + x, cy + y)) {
                    set_pixel(cx + x, cy + y, color);
                }
            }
        }
    } else {
        int x = 0;
        int y = radius;
        int d = 3 - 2 * radius;

        auto plot_pts = [&](int x, int y) {
            if (is_valid_coord(cx + x, cy + y)) set_pixel(cx + x, cy + y, color);
            if (is_valid_coord(cx - x, cy + y)) set_pixel(cx - x, cy + y, color);
            if (is_valid_coord(cx + x, cy - y)) set_pixel(cx + x, cy - y, color);
            if (is_valid_coord(cx - x, cy - y)) set_pixel(cx - x, cy - y, color);
            if (is_valid_coord(cx + y, cy + x)) set_pixel(cx + y, cy + x, color);
            if (is_valid_coord(cx - y, cy + x)) set_pixel(cx - y, cy + x, color);
            if (is_valid_coord(cx + y, cy - x)) set_pixel(cx + y, cy - x, color);
            if (is_valid_coord(cx - y, cy - x)) set_pixel(cx - y, cy - x, color);
        };

        plot_pts(x, y);
        while (y >= x) {
            x++;
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
            plot_pts(x, y);
        }
    }
}

void Sprite::draw_polygon(const std::vector<core::Point>& points, const core::Color& color, bool fill) {
    if (points.size() < 2) return;
    if (points.size() == 2 || !fill) {
        for (size_t i = 0; i < points.size(); ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[(i + 1) % points.size()];
            draw_line(p1.x, p1.y, p2.x, p2.y, color);
        }
        return;
    }

    // Scanline polygon fill
    int min_y = height_, max_y = 0;
    for (const auto& p : points) {
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }
    min_y = std::max(0, min_y);
    max_y = std::min(height_ - 1, max_y);

    for (int y = min_y; y <= max_y; ++y) {
        std::vector<int> node_x;
        size_t n = points.size();
        for (size_t i = 0; i < n; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[(i + 1) % n];
            if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) {
                int x = p1.x + static_cast<int>(std::round(static_cast<double>(y - p1.y) / (p2.y - p1.y) * (p2.x - p1.x)));
                node_x.push_back(x);
            }
        }
        std::sort(node_x.begin(), node_x.end());
        for (size_t i = 0; i + 1 < node_x.size(); i += 2) {
            int start_x = std::max(0, node_x[i]);
            int end_x = std::min(width_ - 1, node_x[i + 1]);
            for (int x = start_x; x <= end_x; ++x) {
                set_pixel(x, y, color);
            }
        }
    }
}

int Sprite::fill_region(int x, int y, const core::Color& new_color) {
    if (!is_valid_coord(x, y)) return 0;
    core::Color target_color = get_pixel(x, y);
    if (target_color == new_color) return 0;

    int filled_count = 0;
    std::vector<bool> visited(static_cast<size_t>(width_ * height_), false);
    std::queue<core::Point> queue;

    queue.push({x, y});
    visited[static_cast<size_t>(y * width_ + x)] = true;

    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};

    while (!queue.empty()) {
        auto pt = queue.front();
        queue.pop();

        set_pixel(pt.x, pt.y, new_color);
        filled_count++;

        for (int i = 0; i < 4; ++i) {
            int nx = pt.x + dx[i];
            int ny = pt.y + dy[i];
            if (is_valid_coord(nx, ny)) {
                size_t idx = static_cast<size_t>(ny * width_ + nx);
                if (!visited[idx] && get_pixel(nx, ny) == target_color) {
                    visited[idx] = true;
                    queue.push({nx, ny});
                }
            }
        }
    }
    return filled_count;
}

int Sprite::replace_color(const core::Color& old_color, const core::Color& new_color, const std::string& region_name) {
    int replaced = 0;
    if (!region_name.empty() && has_region(region_name)) {
        const auto& r = get_region(region_name);
        for (const auto& pt : r.points()) {
            if (is_valid_coord(pt.x, pt.y) && get_pixel(pt.x, pt.y) == old_color) {
                set_pixel(pt.x, pt.y, new_color);
                replaced++;
            }
        }
    } else {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                if (get_pixel(x, y) == old_color) {
                    set_pixel(x, y, new_color);
                    replaced++;
                }
            }
        }
    }
    return replaced;
}

void Sprite::copy_region(const std::string& region_name, int target_x, int target_y) {
    const auto& r = get_region(region_name);
    if (r.empty()) return;

    int dx = target_x - r.bounds().x;
    int dy = target_y - r.bounds().y;

    std::vector<std::pair<core::Point, core::Color>> saved;
    saved.reserve(r.pixel_count());
    for (const auto& pt : r.points()) {
        saved.emplace_back(pt, get_pixel(pt.x, pt.y));
    }

    for (const auto& [pt, color] : saved) {
        int nx = pt.x + dx;
        int ny = pt.y + dy;
        if (is_valid_coord(nx, ny) && !color.is_transparent()) {
            set_pixel(nx, ny, color);
        }
    }
}

void Sprite::move_region(const std::string& region_name, int dx, int dy) {
    if (dx == 0 && dy == 0) return;
    auto& r = get_region(region_name);
    if (r.empty()) return;

    std::vector<std::pair<core::Point, core::Color>> saved;
    saved.reserve(r.pixel_count());
    for (const auto& pt : r.points()) {
        saved.emplace_back(pt, get_pixel(pt.x, pt.y));
    }

    // Erase original
    for (const auto& pt : r.points()) {
        clear_pixel(pt.x, pt.y);
    }

    // Draw at new location and update region points
    std::vector<core::Point> new_points;
    new_points.reserve(saved.size());

    for (const auto& [pt, color] : saved) {
        int nx = pt.x + dx;
        int ny = pt.y + dy;
        if (is_valid_coord(nx, ny)) {
            if (!color.is_transparent()) {
                set_pixel(nx, ny, color);
            }
            new_points.push_back({nx, ny});
        }
    }
    r = Region(region_name, std::move(new_points));
}

void Sprite::erase_region(const std::string& region_name) {
    const auto& r = get_region(region_name);
    for (const auto& pt : r.points()) {
        clear_pixel(pt.x, pt.y);
    }
}

void Sprite::flip_horizontal(const std::string& region_name) {
    if (!region_name.empty() && has_region(region_name)) {
        auto& r = get_region(region_name);
        const auto& bounds = r.bounds();

        std::vector<std::pair<core::Point, core::Color>> saved;
        for (const auto& pt : r.points()) {
            saved.emplace_back(pt, get_pixel(pt.x, pt.y));
            clear_pixel(pt.x, pt.y);
        }

        std::vector<core::Point> new_points;
        for (const auto& [pt, color] : saved) {
            int nx = bounds.x + bounds.right() - pt.x;
            int ny = pt.y;
            if (is_valid_coord(nx, ny)) {
                if (!color.is_transparent()) set_pixel(nx, ny, color);
                new_points.push_back({nx, ny});
            }
        }
        r = Region(region_name, std::move(new_points));
    } else {
        // Whole sprite
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_ / 2; ++x) {
                int rx = width_ - 1 - x;
                auto c1 = get_pixel(x, y);
                auto c2 = get_pixel(rx, y);
                set_pixel(x, y, c2);
                set_pixel(rx, y, c1);
            }
        }
        for (auto& [name, reg] : regions_) {
            reg.mirror_horizontal(width_ / 2);
        }
    }
}

void Sprite::flip_vertical(const std::string& region_name) {
    if (!region_name.empty() && has_region(region_name)) {
        auto& r = get_region(region_name);
        const auto& bounds = r.bounds();

        std::vector<std::pair<core::Point, core::Color>> saved;
        for (const auto& pt : r.points()) {
            saved.emplace_back(pt, get_pixel(pt.x, pt.y));
            clear_pixel(pt.x, pt.y);
        }

        std::vector<core::Point> new_points;
        for (const auto& [pt, color] : saved) {
            int nx = pt.x;
            int ny = bounds.y + bounds.bottom() - pt.y;
            if (is_valid_coord(nx, ny)) {
                if (!color.is_transparent()) set_pixel(nx, ny, color);
                new_points.push_back({nx, ny});
            }
        }
        r = Region(region_name, std::move(new_points));
    } else {
        for (int y = 0; y < height_ / 2; ++y) {
            int ry = height_ - 1 - y;
            for (int x = 0; x < width_; ++x) {
                auto c1 = get_pixel(x, y);
                auto c2 = get_pixel(x, ry);
                set_pixel(x, y, c2);
                set_pixel(x, ry, c1);
            }
        }
        for (auto& [name, reg] : regions_) {
            reg.mirror_vertical(height_ / 2);
        }
    }
}

void Sprite::create_outline(const core::Color& outline_color, const std::string& region_name, bool diagonal) {
    std::vector<core::Point> outline_pts;
    const int dirs4[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    const int dirs8[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};

    const int (*dirs)[2] = diagonal ? dirs8 : dirs4;
    int num_dirs = diagonal ? 8 : 4;

    auto is_inside = [&](int x, int y) {
        if (!is_valid_coord(x, y)) return false;
        if (!region_name.empty() && has_region(region_name)) {
            return get_region(region_name).contains(x, y) && !get_pixel(x, y).is_transparent();
        }
        return !get_pixel(x, y).is_transparent();
    };

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (get_pixel(x, y).is_transparent()) {
                bool borders_inside = false;
                for (int d = 0; d < num_dirs; ++d) {
                    int nx = x + dirs[d][0];
                    int ny = y + dirs[d][1];
                    if (is_inside(nx, ny)) {
                        borders_inside = true;
                        break;
                    }
                }
                if (borders_inside) {
                    outline_pts.push_back({x, y});
                }
            }
        }
    }

    for (const auto& pt : outline_pts) {
        set_pixel(pt.x, pt.y, outline_color);
    }
}

void Sprite::remove_outline(const std::string& region_name) {
    // Finds outermost non-transparent pixels and removes them
    std::vector<core::Point> to_clear;
    const int dirs4[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (!get_pixel(x, y).is_transparent()) {
                if (!region_name.empty() && has_region(region_name) && !get_region(region_name).contains(x, y)) {
                    continue;
                }
                bool touches_transparent = false;
                for (int d = 0; d < 4; ++d) {
                    int nx = x + dirs4[d][0];
                    int ny = y + dirs4[d][1];
                    if (!is_valid_coord(nx, ny) || get_pixel(nx, ny).is_transparent()) {
                        touches_transparent = true;
                        break;
                    }
                }
                if (touches_transparent) {
                    to_clear.push_back({x, y});
                }
            }
        }
    }

    for (const auto& pt : to_clear) {
        clear_pixel(pt.x, pt.y);
    }
}

void Sprite::add_shadow(int dx, int dy, const core::Color& shadow_color, const std::string& region_name) {
    std::vector<core::Point> shadow_pts;

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (!get_pixel(x, y).is_transparent()) {
                if (!region_name.empty() && has_region(region_name) && !get_region(region_name).contains(x, y)) {
                    continue;
                }
                int sx = x + dx;
                int sy = y + dy;
                if (is_valid_coord(sx, sy) && get_pixel(sx, sy).is_transparent()) {
                    shadow_pts.push_back({sx, sy});
                }
            }
        }
    }

    for (const auto& pt : shadow_pts) {
        set_pixel(pt.x, pt.y, shadow_color);
    }
}

void Sprite::add_highlight(int dx, int dy, const core::Color& highlight_color, const std::string& region_name) {
    // Adds highlight on the specified directional edge inside the figure
    std::vector<core::Point> hl_pts;

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (!get_pixel(x, y).is_transparent()) {
                if (!region_name.empty() && has_region(region_name) && !get_region(region_name).contains(x, y)) {
                    continue;
                }
                // Check if opposite side is transparent
                int opp_x = x - dx;
                int opp_y = y - dy;
                if (!is_valid_coord(opp_x, opp_y) || get_pixel(opp_x, opp_y).is_transparent()) {
                    hl_pts.push_back({x, y});
                }
            }
        }
    }

    for (const auto& pt : hl_pts) {
        set_pixel(pt.x, pt.y, highlight_color);
    }
}

void Sprite::quantize_palette(size_t max_colors) {
    palette_.quantize(max_colors);
    // Remap all pixels in all layers to nearest palette color
    for (auto& l : layers_) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                auto c = l.get_pixel(x, y);
                if (!c.is_transparent()) {
                    auto nearest = palette_.find_nearest_color(c, false);
                    l.set_pixel(x, y, nearest);
                }
            }
        }
    }
}

int Sprite::clean_pixel_noise(int neighbor_threshold) {
    std::vector<core::Point> noise;
    const int dirs4[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (!get_pixel(x, y).is_transparent()) {
                int solid_neighbors = 0;
                for (int d = 0; d < 4; ++d) {
                    int nx = x + dirs4[d][0];
                    int ny = y + dirs4[d][1];
                    if (is_valid_coord(nx, ny) && !get_pixel(nx, ny).is_transparent()) {
                        solid_neighbors++;
                    }
                }
                if (solid_neighbors < neighbor_threshold) {
                    noise.push_back({x, y});
                }
            }
        }
    }

    for (const auto& pt : noise) {
        clear_pixel(pt.x, pt.y);
    }
    return static_cast<int>(noise.size());
}

void Sprite::mirror_region(const std::string& region_name, bool horizontal, bool vertical) {
    if (horizontal) flip_horizontal(region_name);
    if (vertical) flip_vertical(region_name);
}

void Sprite::symmetrize_region(const std::string& region_name, char axis) {
    if (!has_region(region_name)) return;
    auto& r = get_region(region_name);
    const auto& bounds = r.bounds();

    if (axis == 'x' || axis == 'X') {
        int mid_x = bounds.x + bounds.width / 2;
        for (int y = bounds.y; y <= bounds.bottom(); ++y) {
            for (int x = bounds.x; x <= mid_x; ++x) {
                int mirror_x = bounds.right() - (x - bounds.x);
                if (is_valid_coord(x, y) && is_valid_coord(mirror_x, y)) {
                    auto c = get_pixel(x, y);
                    set_pixel(mirror_x, y, c);
                    r.add_point(mirror_x, y);
                }
            }
        }
    } else {
        int mid_y = bounds.y + bounds.height / 2;
        for (int y = bounds.y; y <= mid_y; ++y) {
            int mirror_y = bounds.bottom() - (y - bounds.y);
            for (int x = bounds.x; x <= bounds.right(); ++x) {
                if (is_valid_coord(x, y) && is_valid_coord(x, mirror_y)) {
                    auto c = get_pixel(x, y);
                    set_pixel(x, mirror_y, c);
                    r.add_point(x, mirror_y);
                }
            }
        }
    }
    r.recalculate_bounds();
}

std::vector<Region> Sprite::identify_color_clusters() const {
    std::vector<Region> clusters;
    std::vector<bool> visited(static_cast<size_t>(width_ * height_), false);
    const int dirs4[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    int cluster_id = 1;
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            size_t idx = static_cast<size_t>(y * width_ + x);
            if (visited[idx]) continue;
            auto c = get_pixel(x, y);
            if (c.is_transparent()) {
                visited[idx] = true;
                continue;
            }

            // BFS for connected cluster of identical or solid pixels
            std::vector<core::Point> cluster_points;
            std::queue<core::Point> q;
            q.push({x, y});
            visited[idx] = true;

            while (!q.empty()) {
                auto pt = q.front();
                q.pop();
                cluster_points.push_back(pt);

                for (int d = 0; d < 4; ++d) {
                    int nx = pt.x + dirs4[d][0];
                    int ny = pt.y + dirs4[d][1];
                    if (is_valid_coord(nx, ny)) {
                        size_t n_idx = static_cast<size_t>(ny * width_ + nx);
                        if (!visited[n_idx] && !get_pixel(nx, ny).is_transparent()) {
                            visited[n_idx] = true;
                            q.push({nx, ny});
                        }
                    }
                }
            }

            if (cluster_points.size() >= 3) {
                std::string c_name = "cluster_" + std::to_string(cluster_id++);
                clusters.emplace_back(c_name, std::move(cluster_points));
            }
        }
    }
    return clusters;
}

void Sprite::auto_detect_regions() {
    auto clusters = identify_color_clusters();
    for (auto& c : clusters) {
        regions_[c.name()] = std::move(c);
    }
}

core::Rect Sprite::calculate_bounding_box() const {
    return active_layer().calculate_bounding_box();
}

std::vector<core::Color> Sprite::get_unique_colors(bool include_transparent) const {
    std::unordered_set<uint32_t> seen;
    std::vector<core::Color> unique;

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            auto c = get_pixel(x, y);
            if (!include_transparent && c.is_transparent()) continue;
            uint32_t u = c.to_u32();
            if (seen.insert(u).second) {
                unique.push_back(c);
            }
        }
    }
    return unique;
}

std::unordered_map<uint32_t, size_t> Sprite::get_color_histogram() const {
    std::unordered_map<uint32_t, size_t> hist;
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            auto c = get_pixel(x, y);
            if (!c.is_transparent()) {
                hist[c.to_u32()]++;
            }
        }
    }
    return hist;
}

} // namespace pixelforge::sprite
