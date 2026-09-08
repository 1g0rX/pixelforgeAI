#include "animation/FrameDiffer.hpp"
#include <sstream>
#include <unordered_set>

namespace pixelforge::animation {

std::string FrameDiffResult::to_text_format() const {
    std::ostringstream ss;
    ss << "=== FRAME COMPARISON: Frame " << frame_a_index << " -> Frame " << frame_b_index << " ===\n";
    ss << "Pixel differences: " << pixel_difference_count << "\n";
    ss << "Bounding box shift: dx=" << bbox_dx << ", dy=" << bbox_dy << "\n";
    ss << "Palette identical: " << (palette_identical ? "yes" : "no") << "\n";

    ss << "\nChanged regions (" << changed_regions.size() << "):\n";
    if (changed_regions.empty()) {
        ss << "  (none)\n";
    } else {
        for (const auto& r : changed_regions) {
            ss << "  - " << r << "\n";
        }
    }

    ss << "\nUnchanged regions (" << unchanged_regions.size() << "):\n";
    if (unchanged_regions.empty()) {
        ss << "  (none)\n";
    } else {
        for (const auto& r : unchanged_regions) {
            ss << "  - " << r << "\n";
        }
    }
    ss << "===========================================";
    return ss.str();
}

nlohmann::json FrameDiffResult::to_json() const {
    return {
        {"frame_a", frame_a_index},
        {"frame_b", frame_b_index},
        {"pixel_diff_count", pixel_difference_count},
        {"bbox_dx", bbox_dx},
        {"bbox_dy", bbox_dy},
        {"palette_identical", palette_identical},
        {"changed_regions", changed_regions},
        {"unchanged_regions", unchanged_regions}
    };
}

FrameDiffResult FrameDiffer::compare(const Frame& a, const Frame& b, size_t idx_a, size_t idx_b) {
    FrameDiffResult result;
    result.frame_a_index = idx_a;
    result.frame_b_index = idx_b;

    const auto& sa = a.sprite;
    const auto& sb = b.sprite;

    result.bbox_a = sa.calculate_bounding_box();
    result.bbox_b = sb.calculate_bounding_box();
    result.bbox_dx = result.bbox_b.x - result.bbox_a.x;
    result.bbox_dy = result.bbox_b.y - result.bbox_a.y;

    int w = std::min(sa.width(), sb.width());
    int h = std::min(sa.height(), sb.height());

    // Check pixel differences
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (sa.get_pixel(x, y) != sb.get_pixel(x, y)) {
                result.pixel_difference_count++;
            }
        }
    }

    // Check palette equality
    const auto& pal_a = sa.palette().colors();
    const auto& pal_b = sb.palette().colors();
    if (pal_a.size() != pal_b.size()) {
        result.palette_identical = false;
    } else {
        for (size_t i = 0; i < pal_a.size(); ++i) {
            if (pal_a[i] != pal_b[i]) {
                result.palette_identical = false;
                break;
            }
        }
    }

    // Check regions
    std::unordered_set<std::string> all_regions;
    for (const auto& [name, _] : sa.regions()) all_regions.insert(name);
    for (const auto& [name, _] : sb.regions()) all_regions.insert(name);

    for (const auto& rname : all_regions) {
        if (!sa.has_region(rname) || !sb.has_region(rname)) {
            result.changed_regions.push_back(rname);
            continue;
        }

        const auto& ra = sa.get_region(rname);
        const auto& rb = sb.get_region(rname);

        if (ra.bounds() != rb.bounds() || ra.pixel_count() != rb.pixel_count()) {
            result.changed_regions.push_back(rname);
            continue;
        }

        bool diff_pixel = false;
        for (const auto& pt : ra.points()) {
            if (sa.get_pixel(pt.x, pt.y) != sb.get_pixel(pt.x, pt.y)) {
                diff_pixel = true;
                break;
            }
        }

        if (diff_pixel) {
            result.changed_regions.push_back(rname);
        } else {
            result.unchanged_regions.push_back(rname);
        }
    }

    return result;
}

} // namespace pixelforge::animation
