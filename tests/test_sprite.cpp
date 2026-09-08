#include "TestRunner.hpp"
#include "sprite/Sprite.hpp"
#include "core/Error.hpp"

using namespace pixelforge;

TEST(SpriteTest, CreationAndDimensions) {
    sprite::Sprite spr(32, 32, "test");
    ASSERT_EQ(spr.width(), 32);
    ASSERT_EQ(spr.height(), 32);
    ASSERT_EQ(spr.name(), "test");
    ASSERT_EQ(spr.layer_count(), 1);
    ASSERT_TRUE(spr.calculate_bounding_box().is_empty());
}

TEST(SpriteTest, PixelReadWrite) {
    sprite::Sprite spr(16, 16);
    core::Color red{255, 0, 0, 255};
    spr.set_pixel(5, 5, red);

    ASSERT_EQ(spr.get_pixel(5, 5), red);
    ASSERT_TRUE(spr.get_pixel(0, 0).is_transparent());

    spr.clear_pixel(5, 5);
    ASSERT_TRUE(spr.get_pixel(5, 5).is_transparent());
}

TEST(SpriteTest, OutOfBoundsHandling) {
    sprite::Sprite spr(16, 16);
    ASSERT_FALSE(spr.is_valid_coord(-1, 0));
    ASSERT_FALSE(spr.is_valid_coord(16, 0));
    ASSERT_FALSE(spr.is_valid_coord(0, 16));

    // get_pixel out of bounds should return transparent without crash
    ASSERT_TRUE(spr.get_pixel(-5, 100).is_transparent());

    // set_pixel out of bounds throws PixelForgeException
    bool threw = false;
    try {
        spr.set_pixel(20, 20, core::Color{255, 255, 255, 255});
    } catch (const core::PixelForgeException& e) {
        threw = true;
    }
    ASSERT_TRUE(threw);
}

TEST(SpriteTest, DrawLine) {
    sprite::Sprite spr(16, 16);
    core::Color blue{0, 0, 255, 255};
    spr.draw_line(2, 2, 6, 2, blue); // horizontal line

    for (int x = 2; x <= 6; ++x) {
        ASSERT_EQ(spr.get_pixel(x, 2), blue);
    }
    ASSERT_TRUE(spr.get_pixel(1, 2).is_transparent());
    ASSERT_TRUE(spr.get_pixel(7, 2).is_transparent());
}

TEST(SpriteTest, DrawRectangle) {
    sprite::Sprite spr(16, 16);
    core::Color green{0, 255, 0, 255};
    spr.draw_rectangle(3, 3, 4, 4, green, true); // filled 4x4

    for (int y = 3; y < 7; ++y) {
        for (int x = 3; x < 7; ++x) {
            ASSERT_EQ(spr.get_pixel(x, y), green);
        }
    }
    auto bbox = spr.calculate_bounding_box();
    ASSERT_EQ(bbox.x, 3);
    ASSERT_EQ(bbox.y, 3);
    ASSERT_EQ(bbox.width, 4);
    ASSERT_EQ(bbox.height, 4);
}

TEST(SpriteTest, FloodFill) {
    sprite::Sprite spr(16, 16);
    core::Color white{255, 255, 255, 255};
    core::Color yellow{255, 255, 0, 255};

    // Draw enclosed box
    spr.draw_rectangle(2, 2, 6, 6, white, false);
    // Fill interior
    int filled = spr.fill_region(3, 3, yellow);
    ASSERT_EQ(filled, 16); // 4x4 interior
    ASSERT_EQ(spr.get_pixel(3, 3), yellow);
    ASSERT_EQ(spr.get_pixel(2, 2), white);
}

TEST(SpriteTest, MultiLayerBlending) {
    sprite::Sprite spr(8, 8);
    core::Color base_color{100, 100, 100, 255};
    spr.set_pixel(2, 2, base_color);

    size_t l2 = spr.add_layer("Overlay");
    spr.set_active_layer(l2);
    core::Color overlay_color{200, 0, 0, 255};
    spr.set_pixel(2, 2, overlay_color);

    // Active layer pixel is overlay
    ASSERT_EQ(spr.get_pixel(2, 2), overlay_color);
    // Composite pixel combines visible layers (top layer wins)
    ASSERT_EQ(spr.get_composite_pixel(2, 2), overlay_color);
}
