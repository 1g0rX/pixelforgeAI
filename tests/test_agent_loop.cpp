#include "TestRunner.hpp"
#include "agent/AgentLoop.hpp"
#include "agent/MockAgentBackend.hpp"
#include "tools/PrimitiveTools.hpp"
#include "tools/SemanticTools.hpp"
#include "animation/AnimationTools.hpp"

using namespace pixelforge;

TEST(AgentLoopTest, SlimeCreationEndToEnd) {
    auto registry = std::make_shared<tools::ToolRegistry>();
    tools::register_primitive_tools(*registry);
    tools::register_semantic_tools(*registry);
    animation::register_animation_tools(*registry);

    auto backend = std::make_shared<agent::MockAgentBackend>();
    agent::AgentLoop loop(backend, registry);

    sprite::Sprite spr(32, 32);
    sprite::History hist;
    std::shared_ptr<animation::Animation> anim;

    std::string prompt = "Crie um slime verde 32x32 usando no maximo 8 cores";
    std::string final_msg = loop.process_prompt(prompt, spr, hist, anim);

    ASSERT_FALSE(final_msg.empty());
    ASSERT_EQ(spr.width(), 32);
    ASSERT_EQ(spr.height(), 32);

    auto bbox = spr.calculate_bounding_box();
    ASSERT_FALSE(bbox.is_empty());
    ASSERT_TRUE(bbox.width >= 10);
    ASSERT_TRUE(bbox.height >= 10);

    // Verify regions were created
    ASSERT_TRUE(spr.has_region("body"));
    ASSERT_TRUE(spr.has_region("eyes"));

    // Verify colors limit
    auto ucolors = spr.get_unique_colors(false);
    ASSERT_TRUE(ucolors.size() <= 8);
    ASSERT_TRUE(ucolors.size() >= 3);
}

TEST(AgentLoopTest, JumpAnimationEndToEnd) {
    auto registry = std::make_shared<tools::ToolRegistry>();
    tools::register_primitive_tools(*registry);
    tools::register_semantic_tools(*registry);
    animation::register_animation_tools(*registry);

    auto backend = std::make_shared<agent::MockAgentBackend>();
    agent::AgentLoop loop(backend, registry);

    // Prepare a base sprite with body region
    sprite::Sprite spr(32, 32);
    spr.draw_rectangle(10, 10, 12, 12, core::Color{0, 200, 0, 255}, true);
    spr.create_region("body", core::Rect{10, 10, 12, 12});

    sprite::History hist;
    std::shared_ptr<animation::Animation> anim;

    std::string prompt = "Faca uma animacao de pulo com 6 frames";
    std::string final_msg = loop.process_prompt(prompt, spr, hist, anim);

    ASSERT_FALSE(final_msg.empty());
    ASSERT_TRUE(anim != nullptr);
    ASSERT_EQ(anim->frame_count(), 6);

    // Frame 0 and Frame 3 (apex jump) must have different vertical positions
    auto b0 = anim->frame(0).sprite.get_region("body").bounds();
    auto b3 = anim->frame(3).sprite.get_region("body").bounds();
    ASSERT_NE(b0.y, b3.y);
}
