#include "animation/AnimationTools.hpp"
#include "animation/Animation.hpp"
#include "animation/FrameDiffer.hpp"
#include <format>

namespace pixelforge::animation {

// 1. create_animation
class CreateAnimationTool : public tools::ITool {
public:
    std::string name() const override { return "create_animation"; }
    std::string description() const override {
        return "Initializes an animation timeline. Automatically adds the current sprite as Frame 0.";
    }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"name", {{"type", "string"}, {"description", "Animation name (e.g. 'walk', 'jump')"}}},
                {"duration_ms", {{"type", "integer"}, {"description", "Default frame duration in milliseconds (default 100)"}}}
            }}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        std::string anim_name = args.value("name", "anim");
        int duration = args.value("duration_ms", 100);

        ctx.animation = std::make_shared<Animation>(ctx.sprite.width(), ctx.sprite.height(), anim_name);
        ctx.animation->add_frame(ctx.sprite, duration, "frame_0");

        return tools::ToolResult::ok(std::format("Created animation '{}' with initial Frame 0 (duration {}ms)", anim_name, duration));
    }
};

// 2. add_frame
class AddFrameTool : public tools::ITool {
public:
    std::string name() const override { return "add_frame"; }
    std::string description() const override {
        return "Adds current sprite as a new frame in the active animation timeline.";
    }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"duration_ms", {{"type", "integer"}, {"description", "Duration of frame in ms (default 100)"}}},
                {"name", {{"type", "string"}, {"description", "Optional frame label"}}}
            }}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) {
            return tools::ToolResult::fail("No active animation. Call create_animation first.");
        }
        int duration = args.value("duration_ms", 100);
        std::string fname = args.value("name", "frame_" + std::to_string(ctx.animation->frame_count()));
        size_t idx = ctx.animation->add_frame(ctx.sprite, duration, fname);
        return tools::ToolResult::ok(std::format("Added frame {} ('{}', {}ms). Total frames: {}",
                                                 idx, fname, duration, ctx.animation->frame_count()));
    }
};

// 3. duplicate_frame
class DuplicateFrameTool : public tools::ITool {
public:
    std::string name() const override { return "duplicate_frame"; }
    std::string description() const override { return "Duplicates an existing frame in the animation."; }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"frame_index", {{"type", "integer"}, {"description", "Index of frame to duplicate"}}}
            }},
            {"required", {"frame_index"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        int f_idx = args["frame_index"].get<int>();
        if (f_idx < 0 || static_cast<size_t>(f_idx) >= ctx.animation->frame_count()) {
            return tools::ToolResult::fail("Frame index out of bounds");
        }
        size_t new_idx = ctx.animation->duplicate_frame(static_cast<size_t>(f_idx));
        return tools::ToolResult::ok(std::format("Duplicated frame {} to frame {}. Total frames: {}",
                                                 f_idx, new_idx, ctx.animation->frame_count()));
    }
};

// 4. delete_frame
class DeleteFrameTool : public tools::ITool {
public:
    std::string name() const override { return "delete_frame"; }
    std::string description() const override { return "Deletes a frame from the animation."; }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"frame_index", {{"type", "integer"}, {"description", "Index of frame to delete"}}}
            }},
            {"required", {"frame_index"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        int f_idx = args["frame_index"].get<int>();
        if (f_idx < 0 || static_cast<size_t>(f_idx) >= ctx.animation->frame_count()) {
            return tools::ToolResult::fail("Frame index out of bounds");
        }
        bool ok = ctx.animation->delete_frame(static_cast<size_t>(f_idx));
        if (ok) return tools::ToolResult::ok(std::format("Deleted frame {}. Remaining frames: {}", f_idx, ctx.animation->frame_count()));
        return tools::ToolResult::fail("Cannot delete last remaining frame in animation");
    }
};

// 5. reorder_frame
class ReorderFrameTool : public tools::ITool {
public:
    std::string name() const override { return "reorder_frame"; }
    std::string description() const override { return "Moves a frame from one position to another in the timeline."; }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"from_index", {{"type", "integer"}, {"description", "Source index"}}},
                {"to_index", {{"type", "integer"}, {"description", "Destination index"}}}
            }},
            {"required", {"from_index", "to_index"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        int from_i = args["from_index"].get<int>();
        int to_i = args["to_index"].get<int>();
        bool ok = ctx.animation->reorder_frame(static_cast<size_t>(from_i), static_cast<size_t>(to_i));
        if (ok) return tools::ToolResult::ok(std::format("Reordered frame {} to {}", from_i, to_i));
        return tools::ToolResult::fail("Reorder failed: invalid indices");
    }
};

// 6. set_frame_duration
class SetFrameDurationTool : public tools::ITool {
public:
    std::string name() const override { return "set_frame_duration"; }
    std::string description() const override { return "Sets playback duration of a specific frame in milliseconds."; }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"frame_index", {{"type", "integer"}, {"description", "Frame index"}}},
                {"duration_ms", {{"type", "integer"}, {"description", "Duration in milliseconds"}}}
            }},
            {"required", {"frame_index", "duration_ms"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        int f_idx = args["frame_index"].get<int>();
        int dur = args["duration_ms"].get<int>();
        bool ok = ctx.animation->set_frame_duration(static_cast<size_t>(f_idx), dur);
        if (ok) return tools::ToolResult::ok(std::format("Set frame {} duration to {}ms", f_idx, dur));
        return tools::ToolResult::fail("Invalid frame index or duration");
    }
};

// 7. get_frame
class GetFrameTool : public tools::ITool {
public:
    std::string name() const override { return "get_frame"; }
    std::string description() const override {
        return "Loads a frame from the animation into the active editing workspace.";
    }
    bool is_modifying() const override { return false; }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"frame_index", {{"type", "integer"}, {"description", "Index of frame to edit"}}}
            }},
            {"required", {"frame_index"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        int f_idx = args["frame_index"].get<int>();
        if (f_idx < 0 || static_cast<size_t>(f_idx) >= ctx.animation->frame_count()) {
            return tools::ToolResult::fail("Frame index out of bounds");
        }
        ctx.animation->set_current_frame_index(static_cast<size_t>(f_idx));
        ctx.sprite = ctx.animation->frame(static_cast<size_t>(f_idx)).sprite;
        return tools::ToolResult::ok(std::format("Loaded frame {} into editor", f_idx));
    }
};

// 8. update_frame
class UpdateFrameTool : public tools::ITool {
public:
    std::string name() const override { return "update_frame"; }
    std::string description() const override {
        return "Commits the current editor sprite back into the active frame in the animation timeline.";
    }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"frame_index", {{"type", "integer"}, {"description", "Index of frame to update (default current)"}}}
            }}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        size_t f_idx = ctx.animation->current_frame_index();
        if (args.contains("frame_index")) {
            f_idx = static_cast<size_t>(args["frame_index"].get<int>());
        }
        if (f_idx >= ctx.animation->frame_count()) {
            return tools::ToolResult::fail("Frame index out of bounds");
        }
        ctx.animation->frame(f_idx).sprite = ctx.sprite;
        return tools::ToolResult::ok(std::format("Committed editor sprite to animation frame {}", f_idx));
    }
};

// 9. compare_frames
class CompareFramesTool : public tools::ITool {
public:
    std::string name() const override { return "compare_frames"; }
    std::string description() const override {
        return "Compares two animation frames and reports changed/unchanged regions, bounding box shift, and pixel diffs.";
    }
    bool is_modifying() const override { return false; }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"frame_a", {{"type", "integer"}, {"description", "First frame index"}}},
                {"frame_b", {{"type", "integer"}, {"description", "Second frame index"}}}
            }},
            {"required", {"frame_a", "frame_b"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        size_t fa = static_cast<size_t>(args["frame_a"].get<int>());
        size_t fb = static_cast<size_t>(args["frame_b"].get<int>());
        if (fa >= ctx.animation->frame_count() || fb >= ctx.animation->frame_count()) {
            return tools::ToolResult::fail("Frame indices out of bounds");
        }
        auto diff = FrameDiffer::compare(ctx.animation->frame(fa), ctx.animation->frame(fb), fa, fb);
        return tools::ToolResult::ok(diff.to_text_format(), diff.to_json());
    }
};

// 10. copy_region_between_frames
class CopyRegionBetweenFramesTool : public tools::ITool {
public:
    std::string name() const override { return "copy_region_between_frames"; }
    std::string description() const override {
        return "Copies a semantic region from one frame to another frame.";
    }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Region name"}}},
                {"from_frame", {{"type", "integer"}, {"description", "Source frame index"}}},
                {"to_frame", {{"type", "integer"}, {"description", "Destination frame index"}}}
            }},
            {"required", {"region", "from_frame", "to_frame"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        std::string rname = args["region"].get<std::string>();
        size_t fa = static_cast<size_t>(args["from_frame"].get<int>());
        size_t fb = static_cast<size_t>(args["to_frame"].get<int>());
        ctx.animation->copy_region_between_frames(rname, fa, fb);
        return tools::ToolResult::ok(std::format("Copied region '{}' from frame {} to frame {}", rname, fa, fb));
    }
};

// 11. interpolate_region
class InterpolateRegionTool : public tools::ITool {
public:
    std::string name() const override { return "interpolate_region"; }
    std::string description() const override {
        return "Smoothly interpolates the movement of a region between keyframes from_frame and to_frame.";
    }
    std::vector<std::string> required_permissions() const override { return {"animation:edit"}; }
    nlohmann::json parameter_schema() const override {
        return {
            {"type", "object"},
            {"properties", {
                {"region", {{"type", "string"}, {"description", "Region name to interpolate"}}},
                {"from_frame", {{"type", "integer"}, {"description", "Start keyframe index"}}},
                {"to_frame", {{"type", "integer"}, {"description", "End keyframe index"}}}
            }},
            {"required", {"region", "from_frame", "to_frame"}}
        };
    }
    tools::ToolResult execute(tools::ExecutionContext& ctx, const nlohmann::json& args) override {
        if (!ctx.animation) return tools::ToolResult::fail("No active animation.");
        std::string rname = args["region"].get<std::string>();
        size_t fa = static_cast<size_t>(args["from_frame"].get<int>());
        size_t fb = static_cast<size_t>(args["to_frame"].get<int>());
        ctx.animation->interpolate_region(rname, fa, fb);
        return tools::ToolResult::ok(std::format("Interpolated region '{}' across frames {} to {}", rname, fa, fb));
    }
};

void register_animation_tools(tools::ToolRegistry& registry) {
    registry.register_tool(std::make_unique<CreateAnimationTool>());
    registry.register_tool(std::make_unique<AddFrameTool>());
    registry.register_tool(std::make_unique<DuplicateFrameTool>());
    registry.register_tool(std::make_unique<DeleteFrameTool>());
    registry.register_tool(std::make_unique<ReorderFrameTool>());
    registry.register_tool(std::make_unique<SetFrameDurationTool>());
    registry.register_tool(std::make_unique<GetFrameTool>());
    registry.register_tool(std::make_unique<UpdateFrameTool>());
    registry.register_tool(std::make_unique<CompareFramesTool>());
    registry.register_tool(std::make_unique<CopyRegionBetweenFramesTool>());
    registry.register_tool(std::make_unique<InterpolateRegionTool>());
}

} // namespace pixelforge::animation
