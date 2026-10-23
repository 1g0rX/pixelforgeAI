#include "sprite/SpriteState.hpp"
#include <sstream>
#include <iomanip>
#include <map>

namespace pixelforge::sprite {

std::string SpriteStateSummary::to_text_format(bool include_ascii) const {
    std::ostringstream ss;
    ss << "=== SPRITE STATE ===\n";
    ss << "Resolution: " << width << "x" << height << "\n";
    ss << "Colors: " << color_count << "\n";
    ss << "Transparent: " << (has_transparency ? "yes" : "no") << "\n";

    if (bounding_box.is_empty()) {
        ss << "BoundingBox: empty (canvas is blank)\n";
    } else {
        ss << "BoundingBox: x=" << bounding_box.x << ".." << bounding_box.right()
           << " y=" << bounding_box.y << ".." << bounding_box.bottom()
           << " (size " << bounding_box.width << "x" << bounding_box.height << ")\n";
    }

    ss << "\nRegions (" << regions.size() << "):\n";
    if (regions.empty()) {
        ss << "  (none defined)\n";
    } else {
        for (const auto& [rname, rrect] : regions) {
            ss << "  " << rname << ": (" << rrect.x << "," << rrect.y << ")-("
               << rrect.right() << "," << rrect.bottom() << ") [size "
               << rrect.width << "x" << rrect.height << "]\n";
        }
    }

    ss << "\nPalette (" << palette_hex.size() << " colors):\n  ";
    for (size_t i = 0; i < palette_hex.size(); ++i) {
        ss << palette_hex[i] << (i + 1 < palette_hex.size() ? ", " : "\n");
    }

    if (include_ascii && !bounding_box.is_empty() && !ascii_art.empty()) {
        ss << "\nASCII Matrix Preview:\n";
        if (!ascii_legend.empty()) {
            ss << "Legend:\n" << ascii_legend << "\n";
        }
        ss << ascii_art << "\n";
    }
    ss << "====================";
    return ss.str();
}

nlohmann::json SpriteStateSummary::to_json() const {
    nlohmann::json j;
    j["width"] = width;
    j["height"] = height;
    j["has_transparency"] = has_transparency;
    j["color_count"] = color_count;
    j["bounding_box"] = {
        {"x", bounding_box.x},
        {"y", bounding_box.y},
        {"width", bounding_box.width},
        {"height", bounding_box.height}
    };
    nlohmann::json r_json = nlohmann::json::object();
    for (const auto& [name, rect] : regions) {
        r_json[name] = {
            {"x", rect.x}, {"y", rect.y},
            {"width", rect.width}, {"height", rect.height}
        };
    }
    j["regions"] = r_json;
    j["palette"] = palette_hex;
    j["ascii"] = ascii_art;
    j["legend"] = ascii_legend;
    return j;
}

SpriteStateSummary SpriteState::summarize(const Sprite& sprite, bool generate_ascii, int max_ascii_dim) {
    SpriteStateSummary s;
    s.width = sprite.width();
    s.height = sprite.height();
    s.bounding_box = sprite.calculate_bounding_box();

    auto ucolors = sprite.get_unique_colors(false);
    s.color_count = ucolors.size();

    // Check transparency
    s.has_transparency = false;
    for (int y = 0; y < sprite.height(); ++y) {
        for (int x = 0; x < sprite.width(); ++x) {
            if (sprite.get_pixel(x, y).is_transparent()) {
                s.has_transparency = true;
                break;
            }
        }
        if (s.has_transparency) break;
    }

    for (const auto& [name, reg] : sprite.regions()) {
        s.regions[name] = reg.bounds();
    }

    for (const auto& c : sprite.palette().colors()) {
        if (!c.is_transparent()) {
            s.palette_hex.push_back(c.to_hex());
        }
    }

    if (generate_ascii && s.width <= max_ascii_dim && s.height <= max_ascii_dim) {
        s.ascii_art = generate_ascii_grid(sprite, &s.ascii_legend);
    }

    return s;
}

std::string SpriteState::generate_ascii_grid(const Sprite& sprite, std::string* out_legend) {
    const std::string char_palette = "#@%*+=ox123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    auto ucolors = sprite.get_unique_colors(false);
    std::unordered_map<uint32_t, char> color_to_char;
    std::unordered_map<uint32_t, std::string> color_hex;
    std::unordered_map<uint32_t, size_t> color_pixels;

    for (size_t i = 0; i < ucolors.size(); ++i) {
        uint32_t u = ucolors[i].to_u32();
        char symbol = (i < char_palette.size()) ? char_palette[i] : '?';
        color_to_char[u] = symbol;
        color_hex[u] = ucolors[i].to_hex();
    }

    std::ostringstream ss;
    // Top coordinate header
    ss << "    ";
    for (int x = 0; x < sprite.width(); ++x) {
        if (x % 5 == 0) ss << (x / 10);
        else ss << " ";
    }
    ss << "\n    ";
    for (int x = 0; x < sprite.width(); ++x) {
        ss << (x % 10);
    }
    ss << "\n   +" << std::string(static_cast<size_t>(sprite.width()), '-') << "+\n";

    for (int y = 0; y < sprite.height(); ++y) {
        ss << std::setw(2) << y << " |";
        for (int x = 0; x < sprite.width(); ++x) {
            auto c = sprite.get_pixel(x, y);
            if (c.is_transparent()) {
                ss << '.';
            } else {
                uint32_t u = c.to_u32();
                color_pixels[u]++;
                ss << color_to_char[u];
            }
        }
        ss << "|\n";
    }
    ss << "   +" << std::string(static_cast<size_t>(sprite.width()), '-') << "+";

    if (out_legend) {
        std::ostringstream leg;
        leg << "  . = Transparent\n";
        for (const auto& c : ucolors) {
            uint32_t u = c.to_u32();
            leg << "  " << color_to_char[u] << " = " << color_hex[u]
                << " (" << color_pixels[u] << " px)\n";
        }
        *out_legend = leg.str();
    }

    return ss.str();
}

std::string SpriteState::generate_region_ascii(const Sprite& sprite, const std::string& region_name) {
    if (!sprite.has_region(region_name)) {
        return "(region not found)";
    }
    const auto& reg = sprite.get_region(region_name);
    const auto& b = reg.bounds();
    if (b.is_empty()) return "(empty region)";

    std::ostringstream ss;
    ss << "Region [" << region_name << "] " << b.width << "x" << b.height << "\n";
    for (int y = b.y; y <= b.bottom(); ++y) {
        for (int x = b.x; x <= b.right(); ++x) {
            if (reg.contains(x, y)) {
                auto c = sprite.get_pixel(x, y);
                ss << (c.is_transparent() ? '.' : '#');
            } else {
                ss << ' ';
            }
        }
        ss << "\n";
    }
    return ss.str();
}

} // namespace pixelforge::sprite
