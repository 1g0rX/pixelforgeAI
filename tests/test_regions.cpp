#include "TestRunner.hpp"
#include "sprite/Sprite.hpp"

using namespace pixelforge;

TEST(RegionTest, CreateAndQuery) {
    sprite::Sprite spr(32, 32);
    core::Rect r{5, 5, 10, 8};
    spr.create_region("head", r);

    ASSERT_TRUE(spr.has_region("head"));
    ASSERT_FALSE(spr.has_region("body"));

    const auto& reg = spr.get_region("head");
    ASSERT_EQ(reg.name(), "head");
    ASSERT_EQ(reg.bounds().x, 5);
    ASSERT_EQ(reg.bounds().y, 5);
    ASSERT_EQ(reg.bounds().width, 10);
    ASSERT_EQ(reg.bounds().height, 8);
    ASSERT_EQ(reg.pixel_count(), 80);
    ASSERT_TRUE(reg.contains(5, 5));
    ASSERT_TRUE(reg.contains(14, 12));
    ASSERT_FALSE(reg.contains(4, 5));
}

TEST(RegionTest, RenameAndDelete) {
    sprite::Sprite spr(32, 32);
    spr.create_region("arm", core::Rect{2, 2, 4, 4});

    ASSERT_TRUE(spr.rename_region("arm", "left_arm"));
    ASSERT_FALSE(spr.has_region("arm"));
    ASSERT_TRUE(spr.has_region("left_arm"));

    ASSERT_TRUE(spr.delete_region("left_arm"));
    ASSERT_FALSE(spr.has_region("left_arm"));
}

TEST(RegionTest, MoveRegion) {
    sprite::Sprite spr(32, 32);
    core::Color red{255, 0, 0, 255};

    // Draw 3x3 block at (5, 5)
    spr.draw_rectangle(5, 5, 3, 3, red, true);
    spr.create_region("box", core::Rect{5, 5, 3, 3});

    // Move box by dx=+4, dy=+2 -> should be at (9, 7)
    spr.move_region("box", 4, 2);

    // Old location (5, 5) should now be transparent
    ASSERT_TRUE(spr.get_pixel(5, 5).is_transparent());
    // New location (9, 7) should be red
    ASSERT_EQ(spr.get_pixel(9, 7), red);
    ASSERT_EQ(spr.get_pixel(11, 9), red);

    // Region bounds updated
    const auto& reg = spr.get_region("box");
    ASSERT_EQ(reg.bounds().x, 9);
    ASSERT_EQ(reg.bounds().y, 7);
}

TEST(RegionTest, CopyAndEraseRegion) {
    sprite::Sprite spr(32, 32);
    core::Color col{10, 20, 30, 255};
    spr.draw_rectangle(2, 2, 3, 3, col, true);
    spr.create_region("stamp", core::Rect{2, 2, 3, 3});

    // Copy to (10, 10)
    spr.copy_region("stamp", 10, 10);
    ASSERT_EQ(spr.get_pixel(10, 10), col);
    ASSERT_EQ(spr.get_pixel(2, 2), col); // original still there

    // Erase original
    spr.erase_region("stamp");
    ASSERT_TRUE(spr.get_pixel(2, 2).is_transparent());
    ASSERT_EQ(spr.get_pixel(10, 10), col); // copy intact
}
