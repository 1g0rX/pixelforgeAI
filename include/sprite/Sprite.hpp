#pragma once

#include "core/Types.hpp"
#include "sprite/Color.hpp"
#include "sprite/Palette.hpp"
#include "sprite/Layer.hpp"
#include "sprite/Region.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace pixelforge::sprite {

class Sprite {
public:
    Sprite();
    Sprite(int width, int height, std::string name = "untitled",
           core::Color background_color = core::Transparent);

    int width() const { return width_; }
    int height() const { return height_; }
    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    bool is_valid_coord(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    // Layers
    size_t layer_count() const { return layers_.size(); }
    Layer& layer(size_t index);
    const Layer& layer(size_t index) const;
    Layer& active_layer();
    const Layer& active_layer() const;
    size_t active_layer_index() const { return active_layer_index_; }
    void set_active_layer(size_t index);
    size_t add_layer(std::string name, core::Color fill_color = core::Transparent);
    bool remove_layer(size_t index);

    // Palette
    const Palette& palette() const { return palette_; }
    Palette& palette() { return palette_; }
    void set_palette(Palette palette) { palette_ = std::move(palette); }

    // Regions
    const std::unordered_map<std::string, Region>& regions() const { return regions_; }
    bool has_region(const std::string& name) const { return regions_.contains(name); }
    const Region& get_region(const std::string& name) const;
    Region& get_region(const std::string& name);
    void create_region(const std::string& name, const core::Rect& rect);
    void create_region(const std::string& name, std::vector<core::Point> points);
    bool rename_region(const std::string& old_name, const std::string& new_name);
    bool delete_region(const std::string& name);
    void auto_detect_regions();

    // Pixel Access
    core::Color get_pixel(int x, int y) const;
    void set_pixel(int x, int y, const core::Color& color);
    void clear_pixel(int x, int y);
    core::Color get_composite_pixel(int x, int y) const;
    std::vector<core::Color> get_composite_pixels() const;

    // Primitives
    void draw_line(int x0, int y0, int x1, int y1, const core::Color& color);
    void draw_rectangle(int x, int y, int w, int h, const core::Color& color, bool fill = false);
    void draw_circle(int cx, int cy, int radius, const core::Color& color, bool fill = false);
    void draw_polygon(const std::vector<core::Point>& points, const core::Color& color, bool fill = false);
    int fill_region(int x, int y, const core::Color& color);
    int replace_color(const core::Color& old_color, const core::Color& new_color, const std::string& region_name = "");

    // Region Operations
    void copy_region(const std::string& region_name, int target_x, int target_y);
    void move_region(const std::string& region_name, int dx, int dy);
    void erase_region(const std::string& region_name);
    void flip_horizontal(const std::string& region_name = "");
    void flip_vertical(const std::string& region_name = "");

    // Specialized Semantic Tools
    void create_outline(const core::Color& outline_color, const std::string& region_name = "", bool diagonal = false);
    void remove_outline(const std::string& region_name = "");
    void add_shadow(int dx, int dy, const core::Color& shadow_color, const std::string& region_name = "");
    void add_highlight(int dx, int dy, const core::Color& highlight_color, const std::string& region_name = "");
    void quantize_palette(size_t max_colors);
    int clean_pixel_noise(int neighbor_threshold = 1);
    void mirror_region(const std::string& region_name, bool horizontal = true, bool vertical = false);
    void symmetrize_region(const std::string& region_name, char axis = 'x');
    std::vector<Region> identify_color_clusters() const;

    // Analysis Helpers
    core::Rect calculate_bounding_box() const;
    std::vector<core::Color> get_unique_colors(bool include_transparent = false) const;
    std::unordered_map<uint32_t, size_t> get_color_histogram() const;

private:
    int width_;
    int height_;
    std::string name_;
    std::vector<Layer> layers_;
    size_t active_layer_index_ = 0;
    Palette palette_;
    std::unordered_map<std::string, Region> regions_;
};

} // namespace pixelforge::sprite
