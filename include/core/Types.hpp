#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <iomanip>

namespace pixelforge::core {

struct Point {
    int x = 0;
    int y = 0;

    constexpr bool operator==(const Point& other) const = default;
    constexpr Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
    constexpr Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
};

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    constexpr bool operator==(const Rect& other) const = default;

    constexpr int left() const { return x; }
    constexpr int top() const { return y; }
    constexpr int right() const { return x + width - 1; }
    constexpr int bottom() const { return y + height - 1; }

    constexpr bool is_empty() const { return width <= 0 || height <= 0; }

    constexpr bool contains(int px, int py) const {
        return px >= x && px < x + width && py >= y && py < y + height;
    }

    constexpr bool contains(const Point& pt) const {
        return contains(pt.x, pt.y);
    }

    constexpr bool intersects(const Rect& other) const {
        if (is_empty() || other.is_empty()) return false;
        return x < other.x + other.width && x + width > other.x &&
               y < other.y + other.height && y + height > other.y;
    }

    constexpr Rect intersection(const Rect& other) const {
        int nx = std::max(x, other.x);
        int ny = std::max(y, other.y);
        int nw = std::min(x + width, other.x + other.width) - nx;
        int nh = std::min(y + height, other.y + other.height) - ny;
        if (nw <= 0 || nh <= 0) return {0, 0, 0, 0};
        return {nx, ny, nw, nh};
    }

    static Rect from_points(const Point& p1, const Point& p2) {
        int nx = std::min(p1.x, p2.x);
        int ny = std::min(p1.y, p2.y);
        int nw = std::abs(p1.x - p2.x) + 1;
        int nh = std::abs(p1.y - p2.y) + 1;
        return {nx, ny, nw, nh};
    }
};

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    constexpr bool operator==(const Color& other) const = default;

    constexpr bool is_transparent() const {
        return a == 0;
    }

    constexpr uint32_t to_u32() const {
        return (static_cast<uint32_t>(r) << 24) |
               (static_cast<uint32_t>(g) << 16) |
               (static_cast<uint32_t>(b) << 8)  |
               static_cast<uint32_t>(a);
    }

    std::string to_hex(bool include_alpha = false) const {
        std::ostringstream ss;
        ss << '#';
        ss << std::hex << std::setfill('0');
        ss << std::setw(2) << static_cast<int>(r);
        ss << std::setw(2) << static_cast<int>(g);
        ss << std::setw(2) << static_cast<int>(b);
        if (include_alpha) {
            ss << std::setw(2) << static_cast<int>(a);
        }
        return ss.str();
    }

    static Color from_hex(std::string_view hex) {
        if (hex.empty()) return {0, 0, 0, 0};
        if (hex[0] == '#') hex.remove_prefix(1);

        uint32_t val = 0;
        for (char c : hex) {
            val <<= 4;
            if (c >= '0' && c <= '9') val |= (c - '0');
            else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
        }

        if (hex.length() == 3) { // #RGB -> #RRGGBB
            uint8_t r = ((val >> 8) & 0xF) * 17;
            uint8_t g = ((val >> 4) & 0xF) * 17;
            uint8_t b = (val & 0xF) * 17;
            return {r, g, b, 255};
        } else if (hex.length() == 6) { // #RRGGBB
            return {static_cast<uint8_t>((val >> 16) & 0xFF),
                    static_cast<uint8_t>((val >> 8) & 0xFF),
                    static_cast<uint8_t>(val & 0xFF),
                    255};
        } else if (hex.length() == 8) { // #RRGGBBAA
            return {static_cast<uint8_t>((val >> 24) & 0xFF),
                    static_cast<uint8_t>((val >> 16) & 0xFF),
                    static_cast<uint8_t>((val >> 8) & 0xFF),
                    static_cast<uint8_t>(val & 0xFF)};
        }
        return {0, 0, 0, 255};
    }

    constexpr int distance_sq(const Color& other) const {
        int dr = static_cast<int>(r) - static_cast<int>(other.r);
        int dg = static_cast<int>(g) - static_cast<int>(other.g);
        int db = static_cast<int>(b) - static_cast<int>(other.b);
        return dr * dr + dg * dg + db * db;
    }

    constexpr float luminance() const {
        return 0.299f * r + 0.587f * g + 0.114f * b;
    }

    Color blend_over(const Color& bg) const {
        if (a == 255) return *this;
        if (a == 0) return bg;
        float alpha_fg = a / 255.0f;
        float alpha_bg = bg.a / 255.0f;
        float out_a = alpha_fg + alpha_bg * (1.0f - alpha_fg);
        if (out_a <= 0.0f) return {0, 0, 0, 0};

        uint8_t out_r = static_cast<uint8_t>((r * alpha_fg + bg.r * alpha_bg * (1.0f - alpha_fg)) / out_a);
        uint8_t out_g = static_cast<uint8_t>((g * alpha_fg + bg.g * alpha_bg * (1.0f - alpha_fg)) / out_a);
        uint8_t out_b = static_cast<uint8_t>((b * alpha_fg + bg.b * alpha_bg * (1.0f - alpha_fg)) / out_a);
        return {out_r, out_g, out_b, static_cast<uint8_t>(out_a * 255.0f)};
    }
};

inline constexpr Color Transparent{0, 0, 0, 0};
inline constexpr Color Black{0, 0, 0, 255};
inline constexpr Color White{255, 255, 255, 255};

struct ValidationReport {
    bool out_of_bounds = false;
    bool invalid_colors = false;
    bool anti_aliasing_detected = false;
    int isolated_pixel_count = 0;
    int disconnected_regions = 0;
    size_t color_count = 0;
    Rect bounding_box;
    std::vector<std::string> warnings;

    bool is_valid() const {
        return !out_of_bounds && !invalid_colors && !anti_aliasing_detected;
    }

    std::string to_string() const {
        std::ostringstream ss;
        ss << "Validation: out_of_bounds=" << (out_of_bounds ? "true" : "false")
           << ", invalid_colors=" << (invalid_colors ? "true" : "false")
           << ", anti_aliasing=" << (anti_aliasing_detected ? "true" : "false")
           << ", isolated_pixels=" << isolated_pixel_count
           << ", color_count=" << color_count
           << ", bbox=[" << bounding_box.x << "," << bounding_box.y << " "
           << bounding_box.width << "x" << bounding_box.height << "]";
        for (const auto& w : warnings) {
            ss << "\n  - Warning: " << w;
        }
        return ss.str();
    }
};


inline std::ostream& operator<<(std::ostream& os, const Point& pt) {
    return os << "(" << pt.x << ", " << pt.y << ")";
}

inline std::ostream& operator<<(std::ostream& os, const Rect& r) {
    return os << "[" << r.x << ", " << r.y << " " << r.width << "x" << r.height << "]";
}

inline std::ostream& operator<<(std::ostream& os, const Color& c) {
    return os << c.to_hex(true);
}

} // namespace pixelforge::core
