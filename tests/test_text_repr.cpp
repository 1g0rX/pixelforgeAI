#include "TestRunner.hpp"
#include "sprite/Sprite.hpp"
#include "sprite/SpriteState.hpp"

using namespace pixelforge;

TEST(TextRepresentationTest, SummarizeSprite) {
    sprite::Sprite spr(16, 16);
    spr.draw_rectangle(4, 4, 4, 4, core::Color{255, 0, 0, 255}, true);
    spr.create_region("box", core::Rect{4, 4, 4, 4});

    auto summary = sprite::SpriteState::summarize(spr, true);

    ASSERT_EQ(summary.width, 16);
    ASSERT_EQ(summary.height, 16);
    ASSERT_EQ(summary.color_count, 1);
    ASSERT_EQ(summary.bounding_box.x, 4);
    ASSERT_EQ(summary.bounding_box.width, 4);
    ASSERT_TRUE(summary.regions.contains("box"));
    ASSERT_FALSE(summary.ascii_art.empty());

    std::string text_fmt = summary.to_text_format(true);
    ASSERT_TRUE(text_fmt.find("Resolution: 16x16") != std::string::npos);
    ASSERT_TRUE(text_fmt.find("box:") != std::string::npos);
    ASSERT_TRUE(text_fmt.find("ASCII Matrix Preview:") != std::string::npos);

    auto json_fmt = summary.to_json();
    ASSERT_EQ(json_fmt["width"], 16);
    ASSERT_EQ(json_fmt["color_count"], 1);
}
