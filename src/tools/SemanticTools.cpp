#include "tools/SemanticTools.hpp"
#include <format>

namespace pixelforge::tools {

// 1. create_outline
class CreateOutlineTool : public ITool {
public:
    std::string name() const override { return "create_outline"; }
    std::string description() const override {
        return "Generates a clean pixel art outline around solid pixels or a specific named region.";
    }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"color", {{"type", "string"}, {"description", "Hex color for outline (e.g. '#000000')"}}},
                {"region", {{"type", "string"}, {"description", "Optional region name to outline"}}},
                {"diagonal", {{"type", "boolean"}, {"description", "Include 8-way diagonal neighbors (default false)"}}}
            }},
            {"required", {"color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        std::string rname = args.value("region", "");
        bool diag = args.value("diagonal", false);
        ctx.sprite.create_outline(c, rname, diag);
        return ToolResult::ok(std::format("Created outline with color {}", c.to_hex()));
    }
};

// 2. remove_outline
class RemoveOutlineTool : public ITool {
public:
    std::string name() const override { return "remove_outline"; }
    std::string description() const override {
        return "Removes the outermost solid pixel boundary, clearing it to transparent.";
    }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Optional region constraint"}}}
            }}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args.value("region", "");
        ctx.sprite.remove_outline(rname);
        return ToolResult::ok("Removed outermost outline");
    }
};

// 3. add_shadow
class AddShadowTool : public ITool {
public:
    std::string name() const override { return "add_shadow"; }
    std::string description() const override {
        return "Projects a directional shadow (offset dx, dy) behind non-transparent pixels.";
    }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"dx", {{"type", "integer"}, {"description", "Horizontal shadow offset (e.g. 1)"}}},
                {"dy", {{"type", "integer"}, {"description", "Vertical shadow offset (e.g. 1)"}}},
                {"color", {{"type", "string"}, {"description", "Hex color of shadow"}}},
                {"region", {{"type", "string"}, {"description", "Optional region constraint"}}}
            }},
            {"required", {"dx", "dy", "color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int dx = args["dx"].get<int>();
        int dy = args["dy"].get<int>();
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        std::string rname = args.value("region", "");
        ctx.sprite.add_shadow(dx, dy, c, rname);
        return ToolResult::ok(std::format("Added shadow at ({}, {}) with color {}", dx, dy, c.to_hex()));
    }
};

// 4. add_highlight
class AddHighlightTool : public ITool {
public:
    std::string name() const override { return "add_highlight"; }
    std::string description() const override {
        return "Adds highlight along lighting edge (offset dx, dy) inside the silhouette.";
    }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"dx", {{"type", "integer"}, {"description", "Horizontal highlight direction (e.g. -1)"}}},
                {"dy", {{"type", "integer"}, {"description", "Vertical highlight direction (e.g. -1)"}}},
                {"color", {{"type", "string"}, {"description", "Hex color of highlight"}}},
                {"region", {{"type", "string"}, {"description", "Optional region constraint"}}}
            }},
            {"required", {"dx", "dy", "color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int dx = args["dx"].get<int>();
        int dy = args["dy"].get<int>();
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        std::string rname = args.value("region", "");
        ctx.sprite.add_highlight(dx, dy, c, rname);
        return ToolResult::ok(std::format("Added highlight with color {}", c.to_hex()));
    }
};

// 5. quantize_palette
class QuantizePaletteTool : public ITool {
public:
    std::string name() const override { return "quantize_palette"; }
    std::string description() const override {
        return "Reduces the number of unique colors in the sprite to at most max_colors by clustering.";
    }
    std::vector<std::string> required_permissions() const override { return {"palette:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"max_colors", {{"type", "integer"}, {"description", "Maximum number of colors allowed"}}}
            }},
            {"required", {"max_colors"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int max_c = args["max_colors"].get<int>();
        if (max_c < 1) return ToolResult::fail("max_colors must be >= 1");
        ctx.sprite.quantize_palette(static_cast<size_t>(max_c));
        return ToolResult::ok(std::format("Quantized palette to {} colors", max_c));
    }
};

// 6. mirror_region
class MirrorRegionTool : public ITool {
public:
    std::string name() const override { return "mirror_region"; }
    std::string description() const override {
        return "Flips a named region across its own center axes.";
    }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Name of region"}}},
                {"horizontal", {{"type", "boolean"}, {"description", "Mirror horizontally (default true)"}}},
                {"vertical", {{"type", "boolean"}, {"description", "Mirror vertically (default false)"}}}
            }},
            {"required", {"region"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args["region"].get<std::string>();
        if (!ctx.sprite.has_region(rname)) {
            return ToolResult::fail("Region not found: " + rname);
        }
        bool horiz = args.value("horizontal", true);
        bool vert = args.value("vertical", false);
        ctx.sprite.mirror_region(rname, horiz, vert);
        return ToolResult::ok(std::format("Mirrored region '{}' (h={}, v={})", rname, horiz, vert));
    }
};

// 7. symmetrize_region
class SymmetrizeRegionTool : public ITool {
public:
    std::string name() const override { return "symmetrize_region"; }
    std::string description() const override {
        return "Makes a region perfectly symmetrical along axis ('x' for left-to-right symmetry, 'y' for top-to-bottom).";
    }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Name of region"}}},
                {"axis", {{"type", "string"}, {"description", "'x' for horizontal, 'y' for vertical symmetry (default 'x')"}}}
            }},
            {"required", {"region"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args["region"].get<std::string>();
        if (!ctx.sprite.has_region(rname)) {
            return ToolResult::fail("Region not found: " + rname);
        }
        std::string axis_str = args.value("axis", "x");
        char axis = axis_str.empty() ? 'x' : axis_str[0];
        ctx.sprite.symmetrize_region(rname, axis);
        return ToolResult::ok(std::format("Symmetrized region '{}' along axis {}", rname, axis));
    }
};

// 8. clean_pixel_noise
class CleanPixelNoiseTool : public ITool {
public:
    std::string name() const override { return "clean_pixel_noise"; }
    std::string description() const override {
        return "Removes stray orphan pixels (isolated pixels having fewer than threshold solid 4-neighbors).";
    }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"threshold", {{"type", "integer"}, {"description", "Minimum neighbor threshold (default 1)"}}}
            }}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int th = args.value("threshold", 1);
        int cleaned = ctx.sprite.clean_pixel_noise(th);
        return ToolResult::ok(std::format("Cleaned {} stray noise pixel(s)", cleaned));
    }
};

// 9. identify_color_clusters
class IdentifyColorClustersTool : public ITool {
public:
    std::string name() const override { return "identify_color_clusters"; }
    std::string description() const override {
        return "Identifies connected non-transparent color clusters and automatically registers them as semantic regions.";
    }
    std::vector<std::string> required_permissions() const override { return {"region:edit"}; }
    nlohmann::json parameter_schema() const override { return {{"type", "object"}}; }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json&) override {
        auto clusters = ctx.sprite.identify_color_clusters();
        nlohmann::json reg_list = nlohmann::json::array();
        for (auto& c : clusters) {
            reg_list.push_back({
                {"name", c.name()},
                {"pixels", c.pixel_count()},
                {"bounds", {
                    {"x", c.bounds().x}, {"y", c.bounds().y},
                    {"w", c.bounds().width}, {"h", c.bounds().height}
                }}
            });
            ctx.sprite.create_region(c.name(), c.points());
        }
        return ToolResult::ok(std::format("Identified and created {} semantic regions", clusters.size()), reg_list);
    }
};

// 10. create_region
class CreateRegionTool : public ITool {
public:
    std::string name() const override { return "create_region"; }
    std::string description() const override {
        return "Creates a semantic region with a bounding box (x, y, width, height) or point list.";
    }
    std::vector<std::string> required_permissions() const override { return {"region:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"name", {{"type", "string"}, {"description", "Semantic name (e.g. 'head', 'weapon', 'body')"}}},
                {"x", {{"type", "integer"}, {"description", "Bounding box left X"}}},
                {"y", {{"type", "integer"}, {"description", "Bounding box top Y"}}},
                {"width", {{"type", "integer"}, {"description", "Bounding box width"}}},
                {"height", {{"type", "integer"}, {"description", "Bounding box height"}}}
            }},
            {"required", {"name", "x", "y", "width", "height"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args["name"].get<std::string>();
        int x = args["x"].get<int>();
        int y = args["y"].get<int>();
        int w = args["width"].get<int>();
        int h = args["height"].get<int>();
        core::Rect rect{x, y, w, h};
        ctx.sprite.create_region(rname, rect);
        return ToolResult::ok(std::format("Created region '{}' at ({}, {}) {}x{}", rname, x, y, w, h));
    }
};

// 11. rename_region
class RenameRegionTool : public ITool {
public:
    std::string name() const override { return "rename_region"; }
    std::string description() const override { return "Renames an existing semantic region."; }
    std::vector<std::string> required_permissions() const override { return {"region:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"old_name", {{"type", "string"}, {"description", "Current name of region"}}},
                {"new_name", {{"type", "string"}, {"description", "New name for region"}}}
            }},
            {"required", {"old_name", "new_name"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string old_n = args["old_name"].get<std::string>();
        std::string new_n = args["new_name"].get<std::string>();
        bool ok = ctx.sprite.rename_region(old_n, new_n);
        if (ok) return ToolResult::ok(std::format("Renamed region '{}' to '{}'", old_n, new_n));
        return ToolResult::fail("Could not rename region: either old does not exist or new already exists");
    }
};

// 12. delete_region
class DeleteRegionTool : public ITool {
public:
    std::string name() const override { return "delete_region"; }
    std::string description() const override { return "Deletes a semantic region (pixels remain untouched)."; }
    std::vector<std::string> required_permissions() const override { return {"region:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"name", {{"type", "string"}, {"description", "Name of region to delete"}}}
            }},
            {"required", {"name"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args["name"].get<std::string>();
        bool ok = ctx.sprite.delete_region(rname);
        if (ok) return ToolResult::ok("Deleted region: " + rname);
        return ToolResult::fail("Region not found: " + rname);
    }
};

void register_semantic_tools(ToolRegistry& registry) {
    registry.register_tool(std::make_unique<CreateOutlineTool>());
    registry.register_tool(std::make_unique<RemoveOutlineTool>());
    registry.register_tool(std::make_unique<AddShadowTool>());
    registry.register_tool(std::make_unique<AddHighlightTool>());
    registry.register_tool(std::make_unique<QuantizePaletteTool>());
    registry.register_tool(std::make_unique<MirrorRegionTool>());
    registry.register_tool(std::make_unique<SymmetrizeRegionTool>());
    registry.register_tool(std::make_unique<CleanPixelNoiseTool>());
    registry.register_tool(std::make_unique<IdentifyColorClustersTool>());
    registry.register_tool(std::make_unique<CreateRegionTool>());
    registry.register_tool(std::make_unique<RenameRegionTool>());
    registry.register_tool(std::make_unique<DeleteRegionTool>());
}

} // namespace pixelforge::tools
