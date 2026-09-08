#include "TestRunner.hpp"
#include "animation/Animation.hpp"
#include "animation/SpriteSheet.hpp"

using namespace pixelforge;

TEST(SpriteSheetTest, PackingGrid) {
    animation::Animation anim(8, 8, "test_anim");

    // Create 4 distinct frames
    for (int i = 0; i < 4; ++i) {
        sprite::Sprite spr(8, 8);
        core::Color col{static_cast<uint8_t>(50 * i + 50), 0, 0, 255};
        spr.set_pixel(0, 0, col);
        anim.add_frame(spr, 100);
    }

    // Pack into 2x2 sheet with 1px spacing and 2px margin
    animation::SpriteSheetOptions opts{
        .columns = 2,
        .rows = 2,
        .spacing = 1,
        .margin = 2
    };

    auto res = animation::SpriteSheet::generate(anim, opts);

    // Expected width: 2*margin + 2*fw + 1*spacing = 2*2 + 2*8 + 1 = 21
    ASSERT_EQ(res.sheet_sprite.width(), 21);
    ASSERT_EQ(res.sheet_sprite.height(), 21);

    // Verify metadata
    ASSERT_EQ(res.metadata["columns"], 2);
    ASSERT_EQ(res.metadata["rows"], 2);
    ASSERT_EQ(res.metadata["frames"].size(), 4);

    // Frame 0 top-left should be at margin (2, 2)
    ASSERT_EQ(res.sheet_sprite.get_pixel(2, 2), core::Color(50, 0, 0, 255));
    // Frame 1 top-left should be at margin + fw + spacing = 2 + 8 + 1 = 11
    ASSERT_EQ(res.sheet_sprite.get_pixel(11, 2), core::Color(100, 0, 0, 255));
}
