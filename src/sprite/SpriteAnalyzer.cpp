#include "sprite/SpriteAnalyzer.hpp"
#include <queue>
#include <unordered_set>

namespace pixelforge::sprite {

core::ValidationReport SpriteAnalyzer::analyze(const Sprite& sprite, const AnalysisOptions& options) {
    core::ValidationReport report;
    report.bounding_box = sprite.calculate_bounding_box();

    auto ucolors = sprite.get_unique_colors(false);
    report.color_count = ucolors.size();

    // Check bounds
    if (!report.bounding_box.is_empty()) {
        if (report.bounding_box.x < 0 || report.bounding_box.right() >= sprite.width() ||
            report.bounding_box.y < 0 || report.bounding_box.bottom() >= sprite.height()) {
            report.out_of_bounds = true;
            report.warnings.push_back("Content pixels extend beyond sprite boundaries");
        }
    }

    // Check regions bounds
    for (const auto& [rname, reg] : sprite.regions()) {
        const auto& b = reg.bounds();
        if (!b.is_empty()) {
            if (b.x < 0 || b.right() >= sprite.width() || b.y < 0 || b.bottom() >= sprite.height()) {
                report.out_of_bounds = true;
                report.warnings.push_back("Region '" + rname + "' extends beyond sprite boundaries");
            }
        }
    }

    // Check colors count
    if (report.color_count > options.max_allowed_colors) {
        report.invalid_colors = true;
        report.warnings.push_back("Color count (" + std::to_string(report.color_count) +
                                  ") exceeds limit of " + std::to_string(options.max_allowed_colors));
    }

    // Check anti-aliasing (semi-transparent pixels)
    if (!options.allow_semi_transparency) {
        report.anti_aliasing_detected = check_anti_aliasing(sprite);
        if (report.anti_aliasing_detected) {
            report.warnings.push_back("Semi-transparent anti-aliasing artifacts detected");
        }
    }

    // Check isolated noise pixels
    report.isolated_pixel_count = count_isolated_pixels(sprite);
    if (report.isolated_pixel_count > 0) {
        report.warnings.push_back("Found " + std::to_string(report.isolated_pixel_count) +
                                  " isolated pixel(s) (noise)");
    }

    // Check disconnected regions
    report.disconnected_regions = count_disconnected_regions(sprite);
    if (report.disconnected_regions > 0) {
        report.warnings.push_back("Found " + std::to_string(report.disconnected_regions) +
                                  " disconnected semantic region(s)");
    }

    return report;
}

int SpriteAnalyzer::count_isolated_pixels(const Sprite& sprite) {
    int isolated = 0;
    const int dirs4[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (int y = 0; y < sprite.height(); ++y) {
        for (int x = 0; x < sprite.width(); ++x) {
            auto c = sprite.get_pixel(x, y);
            if (!c.is_transparent()) {
                int solid_neighbors = 0;
                for (int d = 0; d < 4; ++d) {
                    int nx = x + dirs4[d][0];
                    int ny = y + dirs4[d][1];
                    if (sprite.is_valid_coord(nx, ny) && !sprite.get_pixel(nx, ny).is_transparent()) {
                        solid_neighbors++;
                    }
                }
                if (solid_neighbors == 0) {
                    isolated++;
                }
            }
        }
    }
    return isolated;
}

int SpriteAnalyzer::count_disconnected_regions(const Sprite& sprite) {
    int disconnected_count = 0;
    const int dirs4[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (const auto& [name, reg] : sprite.regions()) {
        if (reg.pixel_count() <= 1) continue;

        std::unordered_set<core::Point, PointHash> remaining(reg.points().begin(), reg.points().end());
        int components = 0;

        while (!remaining.empty()) {
            components++;
            auto start = *remaining.begin();
            remaining.erase(remaining.begin());

            std::queue<core::Point> q;
            q.push(start);

            while (!q.empty()) {
                auto curr = q.front();
                q.pop();

                for (int d = 0; d < 4; ++d) {
                    core::Point neighbor{curr.x + dirs4[d][0], curr.y + dirs4[d][1]};
                    if (remaining.contains(neighbor)) {
                        remaining.erase(neighbor);
                        q.push(neighbor);
                    }
                }
            }
        }

        if (components > 1) {
            disconnected_count++;
        }
    }

    return disconnected_count;
}

bool SpriteAnalyzer::check_anti_aliasing(const Sprite& sprite) {
    for (int y = 0; y < sprite.height(); ++y) {
        for (int x = 0; x < sprite.width(); ++x) {
            auto c = sprite.get_pixel(x, y);
            if (c.a > 0 && c.a < 255) {
                return true;
            }
        }
    }
    return false;
}

} // namespace pixelforge::sprite
