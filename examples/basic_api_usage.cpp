#include "sprite/Sprite.hpp"
#include "sprite/History.hpp"
#include "tools/ToolRegistry.hpp"
#include "tools/PrimitiveTools.hpp"
#include "tools/SemanticTools.hpp"
#include "animation/Animation.hpp"
#include "animation/AnimationTools.hpp"
#include "io/PngIO.hpp"
#include "io/GifIO.hpp"
#include "io/SpriteSheetIO.hpp"
#include <iostream>

using namespace pixelforge;

int main() {
    std::cout << "=== PixelForge AI - C++ Library Embedding Example ===\n";

    // 1. Initialize Sprite and Tool Registry
    sprite::Sprite spr(16, 16, "hero");
    sprite::History hist;
    tools::ToolRegistry reg;
    tools::register_primitive_tools(reg);
    tools::register_semantic_tools(reg);
    animation::register_animation_tools(reg);

    tools::ExecutionContext ctx{.sprite = spr, .history = hist};

    // 2. Execute primitive & semantic tools via ToolRegistry
    reg.execute_tool("set_palette", ctx, {{"preset", "pico8"}});
    reg.execute_tool("draw_rectangle", ctx, {
        {"x", 4}, {"y", 4}, {"width", 8}, {"height", 8},
        {"color", "#00E436"}, {"fill", true}
    });
    reg.execute_tool("create_region", ctx, {
        {"name", "body"}, {"x", 4}, {"y", 4}, {"width", 8}, {"height", 8}
    });
    reg.execute_tool("create_outline", ctx, {{"color", "#000000"}});

    // 3. Save PNG
    io::PngIO::save(spr, "hero_example.png");
    std::cout << "Saved hero_example.png\n";

    // 4. Create Animation
    auto anim = std::make_shared<animation::Animation>(16, 16, "hero_bob");
    ctx.animation = anim;
    reg.execute_tool("create_animation", ctx, {{"name", "hero_bob"}, {"duration_ms", 150}});
    reg.execute_tool("duplicate_frame", ctx, {{"frame_index", 0}});
    reg.execute_tool("get_frame", ctx, {{"frame_index", 1}});
    reg.execute_tool("move_region", ctx, {{"region", "body"}, {"dx", 0}, {"dy", 1}});
    reg.execute_tool("update_frame", ctx, {{"frame_index", 1}});

    io::GifIO::save(*ctx.animation, "hero_bob.gif");
    std::cout << "Saved hero_bob.gif with " << ctx.animation->frame_count() << " frames\n";

    return 0;
}
