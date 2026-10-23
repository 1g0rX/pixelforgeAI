#include "tools/PrimitiveTools.hpp"
#include "sprite/SpriteState.hpp"
#include "core/Config.hpp"
#include <format>

namespace pixelforge::tools {

// 1. create_sprite
class CreateSpriteTool : public ITool {
public:
    std::string name() const override { return "create_sprite"; }
    std::string description() const override {
        return "Initializes or recreates the sprite canvas with specified width, height, and optional background color.";
    }
    std::vector<std::string> required_permissions() const override { return {"sprite:create"}; }
    nlohmann::json parameter_schema() const override {
        auto cfg = core::Config::discover_and_load();
        return {
            {"type", "object"},
            {"properties", {
                {"width", {{"type", "integer"}, {"description",
                    std::format("Width of the sprite in pixels (1-{}, e.g. 16, 32, 64)", cfg.max_sprite_width)}}},
                {"height", {{"type", "integer"}, {"description",
                    std::format("Height of the sprite in pixels (1-{}, e.g. 16, 32, 64)", cfg.max_sprite_height)}}},
                {"name", {{"type", "string"}, {"description", "Name of the sprite"}}},
                {"background_color", {{"type", "string"}, {"description", "Hex background color (optional, default transparent '#00000000')"}}}
            }},
            {"required", {"width", "height"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        auto cfg = core::Config::discover_and_load();
        int max_w = cfg.max_sprite_width;
        int max_h = cfg.max_sprite_height;

        int w = args["width"].get<int>();
        int h = args["height"].get<int>();
        std::string sname = args.value("name", "sprite");
        core::Color bg = core::Transparent;
        if (args.contains("background_color")) {
            bg = core::Color::from_hex(args["background_color"].get<std::string>());
        }

        if (w <= 0 || h <= 0 || w > max_w || h > max_h) {
            return ToolResult::fail(std::format("Dimensions must be between 1x1 and {}x{}", max_w, max_h));
        }

        ctx.sprite = sprite::Sprite(w, h, sname, bg);
        return ToolResult::ok(std::format("Created sprite '{}' {}x{}", sname, w, h));
    }
};

// 2. get_sprite_info
class GetSpriteInfoTool : public ITool {
public:
    std::string name() const override { return "get_sprite_info"; }
    std::string description() const override {
        return "Returns structured state information about the sprite: resolution, palette, bounding box, regions, and compact ASCII art.";
    }
    bool is_modifying() const override { return false; }
    std::vector<std::string> required_permissions() const override { return {"sprite:read"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"include_ascii", {{"type", "boolean"}, {"description", "Whether to include the compact ASCII preview (default true)"}}}
            }}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        bool inc_ascii = args.value("include_ascii", true);
        auto summary = sprite::SpriteState::summarize(ctx.sprite, inc_ascii);
        return ToolResult::ok(summary.to_text_format(inc_ascii), summary.to_json());
    }
};

// 3. get_pixel
class GetPixelTool : public ITool {
public:
    std::string name() const override { return "get_pixel"; }
    std::string description() const override { return "Gets the color of a pixel at (x, y)."; }
    bool is_modifying() const override { return false; }
    std::vector<std::string> required_permissions() const override { return {"sprite:read"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"x", {{"type", "integer"}, {"description", "X coordinate"}}},
                {"y", {{"type", "integer"}, {"description", "Y coordinate"}}}
            }},
            {"required", {"x", "y"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int x = args["x"].get<int>();
        int y = args["y"].get<int>();
        if (!ctx.sprite.is_valid_coord(x, y)) {
            return ToolResult::fail(std::format("Coordinates ({}, {}) out of bounds", x, y));
        }
        auto c = ctx.sprite.get_pixel(x, y);
        nlohmann::json res;
        res["color"] = c.to_hex(true);
        res["is_transparent"] = c.is_transparent();
        return ToolResult::ok(std::format("Pixel ({}, {}) is {}", x, y, c.to_hex(true)), res);
    }
};

// 4. set_pixel
class SetPixelTool : public ITool {
public:
    std::string name() const override { return "set_pixel"; }
    std::string description() const override { return "Sets the pixel at (x, y) to a specific hex color."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"x", {{"type", "integer"}, {"description", "X coordinate"}}},
                {"y", {{"type", "integer"}, {"description", "Y coordinate"}}},
                {"color", {{"type", "string"}, {"description", "Hex color string (e.g. '#FF0000', '#346524')"}}}
            }},
            {"required", {"x", "y", "color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int x = args["x"].get<int>();
        int y = args["y"].get<int>();
        if (!ctx.sprite.is_valid_coord(x, y)) {
            return ToolResult::fail(std::format("Coordinates ({}, {}) out of bounds (dimensions {}x{})", x, y, ctx.sprite.width(), ctx.sprite.height()));
        }
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        ctx.sprite.set_pixel(x, y, c);
        return ToolResult::ok(std::format("Set pixel ({}, {}) to {}", x, y, c.to_hex()));
    }
};

// 5. clear_pixel
class ClearPixelTool : public ITool {
public:
    std::string name() const override { return "clear_pixel"; }
    std::string description() const override { return "Clears the pixel at (x, y) to transparent."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"x", {{"type", "integer"}, {"description", "X coordinate"}}},
                {"y", {{"type", "integer"}, {"description", "Y coordinate"}}}
            }},
            {"required", {"x", "y"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int x = args["x"].get<int>();
        int y = args["y"].get<int>();
        if (!ctx.sprite.is_valid_coord(x, y)) {
            return ToolResult::fail(std::format("Coordinates ({}, {}) out of bounds", x, y));
        }
        ctx.sprite.clear_pixel(x, y);
        return ToolResult::ok(std::format("Cleared pixel ({}, {})", x, y));
    }
};

// 6. draw_line
class DrawLineTool : public ITool {
public:
    std::string name() const override { return "draw_line"; }
    std::string description() const override { return "Draws a line between (x0, y0) and (x1, y1) with Bresenham's algorithm."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"x0", {{"type", "integer"}, {"description", "Start X"}}},
                {"y0", {{"type", "integer"}, {"description", "Start Y"}}},
                {"x1", {{"type", "integer"}, {"description", "End X"}}},
                {"y1", {{"type", "integer"}, {"description", "End Y"}}},
                {"color", {{"type", "string"}, {"description", "Hex color"}}}
            }},
            {"required", {"x0", "y0", "x1", "y1", "color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int x0 = args["x0"].get<int>();
        int y0 = args["y0"].get<int>();
        int x1 = args["x1"].get<int>();
        int y1 = args["y1"].get<int>();
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        ctx.sprite.draw_line(x0, y0, x1, y1, c);
        return ToolResult::ok(std::format("Drew line from ({}, {}) to ({}, {}) with color {}", x0, y0, x1, y1, c.to_hex()));
    }
};

// 7. draw_rectangle
class DrawRectangleTool : public ITool {
public:
    std::string name() const override { return "draw_rectangle"; }
    std::string description() const override { return "Draws an outlined or filled rectangle."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"x", {{"type", "integer"}, {"description", "Top-left X"}}},
                {"y", {{"type", "integer"}, {"description", "Top-left Y"}}},
                {"width", {{"type", "integer"}, {"description", "Rectangle width"}}},
                {"height", {{"type", "integer"}, {"description", "Rectangle height"}}},
                {"color", {{"type", "string"}, {"description", "Hex color"}}},
                {"fill", {{"type", "boolean"}, {"description", "Whether to fill the rectangle (default false)"}}}
            }},
            {"required", {"x", "y", "width", "height", "color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int x = args["x"].get<int>();
        int y = args["y"].get<int>();
        int w = args["width"].get<int>();
        int h = args["height"].get<int>();
        bool fill = args.value("fill", false);
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        ctx.sprite.draw_rectangle(x, y, w, h, c, fill);
        return ToolResult::ok(std::format("Drew {} rectangle at ({}, {}) {}x{} with color {}",
                                          fill ? "filled" : "outlined", x, y, w, h, c.to_hex()));
    }
};

// 8. draw_polygon
class DrawPolygonTool : public ITool {
public:
    std::string name() const override { return "draw_polygon"; }
    std::string description() const override { return "Draws a polygon connecting a sequence of [x, y] vertices."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"points", {
                    {"type", "array"},
                    {"description", "Array of [x, y] coordinates, e.g. [[10, 5], [20, 5], [15, 15]]"}
                }},
                {"color", {{"type", "string"}, {"description", "Hex color"}}},
                {"fill", {{"type", "boolean"}, {"description", "Whether to fill polygon (default false)"}}}
            }},
            {"required", {"points", "color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::vector<core::Point> pts;
        for (const auto& item : args["points"]) {
            if (item.is_array() && item.size() >= 2) {
                pts.push_back({item[0].get<int>(), item[1].get<int>()});
            } else if (item.is_object() && item.contains("x") && item.contains("y")) {
                pts.push_back({item["x"].get<int>(), item["y"].get<int>()});
            }
        }
        if (pts.size() < 2) {
            return ToolResult::fail("At least 2 points required for a polygon");
        }
        bool fill = args.value("fill", false);
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        ctx.sprite.draw_polygon(pts, c, fill);
        return ToolResult::ok(std::format("Drew polygon with {} vertices", pts.size()));
    }
};

// 9. fill_region (flood fill)
class FillRegionTool : public ITool {
public:
    std::string name() const override { return "fill_region"; }
    std::string description() const override { return "Performs a 4-connected flood fill starting at (x, y)."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"x", {{"type", "integer"}, {"description", "Seed X coordinate"}}},
                {"y", {{"type", "integer"}, {"description", "Seed Y coordinate"}}},
                {"color", {{"type", "string"}, {"description", "New hex color"}}}
            }},
            {"required", {"x", "y", "color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        int x = args["x"].get<int>();
        int y = args["y"].get<int>();
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        int filled = ctx.sprite.fill_region(x, y, c);
        return ToolResult::ok(std::format("Flood filled {} pixels starting from ({}, {})", filled, x, y));
    }
};

// 10. copy_region
class CopyRegionTool : public ITool {
public:
    std::string name() const override { return "copy_region"; }
    std::string description() const override { return "Copies a named semantic region to a new top-left location (target_x, target_y)."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Name of region to copy"}}},
                {"target_x", {{"type", "integer"}, {"description", "Target X"}}},
                {"target_y", {{"type", "integer"}, {"description", "Target Y"}}}
            }},
            {"required", {"region", "target_x", "target_y"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args["region"].get<std::string>();
        int tx = args["target_x"].get<int>();
        int ty = args["target_y"].get<int>();
        if (!ctx.sprite.has_region(rname)) {
            return ToolResult::fail("Region not found: " + rname);
        }
        ctx.sprite.copy_region(rname, tx, ty);
        return ToolResult::ok(std::format("Copied region '{}' to ({}, {})", rname, tx, ty));
    }
};

// 11. move_region
class MoveRegionTool : public ITool {
public:
    std::string name() const override { return "move_region"; }
    std::string description() const override { return "Moves a named semantic region by offset (dx, dy). Original pixels are cleared."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Name of region to move"}}},
                {"dx", {{"type", "integer"}, {"description", "Horizontal offset in pixels"}}},
                {"dy", {{"type", "integer"}, {"description", "Vertical offset in pixels"}}}
            }},
            {"required", {"region", "dx", "dy"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args["region"].get<std::string>();
        int dx = args["dx"].get<int>();
        int dy = args["dy"].get<int>();
        if (!ctx.sprite.has_region(rname)) {
            return ToolResult::fail("Region not found: " + rname);
        }
        ctx.sprite.move_region(rname, dx, dy);
        ToolResult res = ToolResult::ok(std::format("Moved region '{}' by ({}, {})", rname, dx, dy));
        res.modified_regions.push_back(rname);
        return res;
    }
};

// 12. erase_region
class EraseRegionTool : public ITool {
public:
    std::string name() const override { return "erase_region"; }
    std::string description() const override { return "Clears all pixels belonging to a named semantic region."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Name of region to erase"}}}
            }},
            {"required", {"region"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args["region"].get<std::string>();
        if (!ctx.sprite.has_region(rname)) {
            return ToolResult::fail("Region not found: " + rname);
        }
        ctx.sprite.erase_region(rname);
        ToolResult res = ToolResult::ok("Erased region: " + rname);
        res.modified_regions.push_back(rname);
        return res;
    }
};

// 13. replace_color
class ReplaceColorTool : public ITool {
public:
    std::string name() const override { return "replace_color"; }
    std::string description() const override { return "Replaces all occurrences of old_color with new_color, optionally restricted to a region."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"old_color", {{"type", "string"}, {"description", "Old hex color"}}},
                {"new_color", {{"type", "string"}, {"description", "New hex color"}}},
                {"region", {{"type", "string"}, {"description", "Optional region constraint"}}}
            }},
            {"required", {"old_color", "new_color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        core::Color old_c = core::Color::from_hex(args["old_color"].get<std::string>());
        core::Color new_c = core::Color::from_hex(args["new_color"].get<std::string>());
        std::string rname = args.value("region", "");
        int count = ctx.sprite.replace_color(old_c, new_c, rname);
        return ToolResult::ok(std::format("Replaced {} pixel(s) from {} to {}", count, old_c.to_hex(), new_c.to_hex()));
    }
};

// 14. set_palette
class SetPaletteTool : public ITool {
public:
    std::string name() const override { return "set_palette"; }
    std::string description() const override {
        return "Sets active palette using preset name ('pico8', 'gameboy', 'dawnbringer16', 'dawnbringer32', 'endesga32') or a list of hex colors.";
    }
    std::vector<std::string> required_permissions() const override { return {"palette:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"preset", {{"type", "string"}, {"description", "Preset palette name"}}},
                {"colors", {{"type", "array"}, {"description", "List of hex color strings"}}}
            }}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        if (args.contains("preset")) {
            std::string preset = args["preset"].get<std::string>();
            if (preset == "pico8") ctx.sprite.set_palette(sprite::Palette::pico8());
            else if (preset == "gameboy") ctx.sprite.set_palette(sprite::Palette::gameboy());
            else if (preset == "dawnbringer16" || preset == "db16") ctx.sprite.set_palette(sprite::Palette::dawnbringer16());
            else if (preset == "dawnbringer32" || preset == "db32") ctx.sprite.set_palette(sprite::Palette::dawnbringer32());
            else if (preset == "endesga32" || preset == "edg32") ctx.sprite.set_palette(sprite::Palette::endesga32());
            else return ToolResult::fail("Unknown preset: " + preset);
            return ToolResult::ok("Set palette to preset: " + preset);
        } else if (args.contains("colors") && args["colors"].is_array()) {
            std::vector<core::Color> cols;
            cols.push_back(core::Transparent);
            for (const auto& item : args["colors"]) {
                cols.push_back(core::Color::from_hex(item.get<std::string>()));
            }
            ctx.sprite.set_palette(sprite::Palette(std::move(cols)));
            return ToolResult::ok(std::format("Set custom palette with {} colors", ctx.sprite.palette().size()));
        }
        return ToolResult::fail("Must provide 'preset' or 'colors'");
    }
};

// 15. add_color
class AddColorTool : public ITool {
public:
    std::string name() const override { return "add_color"; }
    std::string description() const override { return "Adds a color to the active palette."; }
    std::vector<std::string> required_permissions() const override { return {"palette:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"color", {{"type", "string"}, {"description", "Hex color string"}}}
            }},
            {"required", {"color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        size_t idx = ctx.sprite.palette().add_color(c);
        return ToolResult::ok(std::format("Added color {} to palette at index {}", c.to_hex(), idx));
    }
};

// 16. remove_color
class RemoveColorTool : public ITool {
public:
    std::string name() const override { return "remove_color"; }
    std::string description() const override { return "Removes a color from the active palette."; }
    std::vector<std::string> required_permissions() const override { return {"palette:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"color", {{"type", "string"}, {"description", "Hex color string"}}}
            }},
            {"required", {"color"}}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        core::Color c = core::Color::from_hex(args["color"].get<std::string>());
        bool rem = ctx.sprite.palette().remove_color(c);
        if (rem) return ToolResult::ok("Removed color " + c.to_hex());
        return ToolResult::fail("Color not found in palette: " + c.to_hex());
    }
};

// 17. flip_horizontal
class FlipHorizontalTool : public ITool {
public:
    std::string name() const override { return "flip_horizontal"; }
    std::string description() const override { return "Flips the sprite or a specific region horizontally."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Optional region to flip. If omitted, flips whole sprite."}}}
            }}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args.value("region", "");
        ctx.sprite.flip_horizontal(rname);
        return ToolResult::ok(rname.empty() ? "Flipped entire sprite horizontally" : "Flipped region '" + rname + "' horizontally");
    }
};

// 18. flip_vertical
class FlipVerticalTool : public ITool {
public:
    std::string name() const override { return "flip_vertical"; }
    std::string description() const override { return "Flips the sprite or a specific region vertically."; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Optional region to flip. If omitted, flips whole sprite."}}}
            }}
        };
    }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string rname = args.value("region", "");
        ctx.sprite.flip_vertical(rname);
        return ToolResult::ok(rname.empty() ? "Flipped entire sprite vertically" : "Flipped region '" + rname + "' vertically");
    }
};

// 19. undo
class UndoTool : public ITool {
public:
    std::string name() const override { return "undo"; }
    std::string description() const override { return "Undoes the last operation."; }
    bool is_modifying() const override { return false; }
    std::vector<std::string> required_permissions() const override { return {"history:edit"}; }
    nlohmann::json parameter_schema() const override { return {{"type", "object"}}; }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json&) override {
        if (!ctx.history.can_undo()) {
            return ToolResult::fail("Nothing to undo");
        }
        ctx.sprite = ctx.history.undo(ctx.sprite);
        return ToolResult::ok("Undid last operation successfully");
    }
};

// 20. redo
class RedoTool : public ITool {
public:
    std::string name() const override { return "redo"; }
    std::string description() const override { return "Redoes the previously undone operation."; }
    bool is_modifying() const override { return false; }
    std::vector<std::string> required_permissions() const override { return {"history:edit"}; }
    nlohmann::json parameter_schema() const override { return {{"type", "object"}}; }
    ToolResult execute(ExecutionContext& ctx, const nlohmann::json&) override {
        if (!ctx.history.can_redo()) {
            return ToolResult::fail("Nothing to redo");
        }
        ctx.sprite = ctx.history.redo(ctx.sprite);
        return ToolResult::ok("Redid operation successfully");
    }
};

void register_primitive_tools(ToolRegistry& registry) {
    registry.register_tool(std::make_unique<CreateSpriteTool>());
    registry.register_tool(std::make_unique<GetSpriteInfoTool>());
    registry.register_tool(std::make_unique<GetPixelTool>());
    registry.register_tool(std::make_unique<SetPixelTool>());
    registry.register_tool(std::make_unique<ClearPixelTool>());
    registry.register_tool(std::make_unique<DrawLineTool>());
    registry.register_tool(std::make_unique<DrawRectangleTool>());
    registry.register_tool(std::make_unique<DrawPolygonTool>());
    registry.register_tool(std::make_unique<FillRegionTool>());
    registry.register_tool(std::make_unique<CopyRegionTool>());
    registry.register_tool(std::make_unique<MoveRegionTool>());
    registry.register_tool(std::make_unique<EraseRegionTool>());
    registry.register_tool(std::make_unique<ReplaceColorTool>());
    registry.register_tool(std::make_unique<SetPaletteTool>());
    registry.register_tool(std::make_unique<AddColorTool>());
    registry.register_tool(std::make_unique<RemoveColorTool>());
    registry.register_tool(std::make_unique<FlipHorizontalTool>());
    registry.register_tool(std::make_unique<FlipVerticalTool>());
    registry.register_tool(std::make_unique<UndoTool>());
    registry.register_tool(std::make_unique<RedoTool>());
}

} // namespace pixelforge::tools
