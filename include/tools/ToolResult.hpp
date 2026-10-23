#pragma once

#include "core/Types.hpp"
#include "third_party/nlohmann/json.hpp"
#include <string>
#include <vector>

namespace pixelforge::tools {

struct ToolResult {
    bool success = true;
    std::string message;
    nlohmann::json data;
    core::ValidationReport analysis;
    std::vector<std::string> modified_regions;

    static ToolResult ok(std::string message, nlohmann::json data = nlohmann::json::object()) {
        return ToolResult{
            .success = true,
            .message = std::move(message),
            .data = std::move(data),
            .analysis = {},
            .modified_regions = {}
        };
    }

    static ToolResult fail(std::string message, nlohmann::json data = nlohmann::json::object()) {
        return ToolResult{
            .success = false,
            .message = std::move(message),
            .data = std::move(data),
            .analysis = {},
            .modified_regions = {}
        };
    }

    std::string to_string(bool compact = false) const {
        std::string res = success ? "[SUCCESS] " : "[FAILED] ";
        res += message;
        if (!data.empty()) {
            res += "\nData: " + data.dump();
        }
        if (compact) {
            if (!analysis.is_valid() || !analysis.warnings.empty() || analysis.isolated_pixel_count > 0) {
                res += "\n" + analysis.to_string();
            } else {
                res += " (valid, bbox=[" + std::to_string(analysis.bounding_box.x) + "," +
                       std::to_string(analysis.bounding_box.y) + " " +
                       std::to_string(analysis.bounding_box.width) + "x" +
                       std::to_string(analysis.bounding_box.height) + "], colors=" +
                       std::to_string(analysis.color_count) + ")";
            }
        } else {
            res += "\n" + analysis.to_string();
        }
        return res;
    }
};

} // namespace pixelforge::tools
