#include "TestRunner.hpp"
#include "sprite/Sprite.hpp"

using namespace pixelforge;

TEST(SemanticToolsTest, CreateOutline) {
    sprite::Sprite spr(16, 16);
    core::Color fill{0, 255, 0, 255};
    core::Color outline{0, 0, 0, 255};

    // Draw 2x2 solid block at (5, 5) -> (5,5), (6,5), (5,6), (6,6)
    spr.draw_rectangle(5, 5, 2, 2, fill, true);
    spr.create_outline(outline, "", false);

    // Pixels around block must be outline
    ASSERT_EQ(spr.get_pixel(4, 5), outline);
    ASSERT_EQ(spr.get_pixel(7, 5), outline);
    ASSERT_EQ(spr.get_pixel(5, 4), outline);
    ASSERT_EQ(spr.get_pixel(5, 7), outline);

    // Inner block remains fill
    ASSERT_EQ(spr.get_pixel(5, 5), fill);
}

TEST(SemanticToolsTest, AddShadow) {
    sprite::Sprite spr(16, 16);
    core::Color fill{100, 100, 200, 255};
    core::Color shadow{20, 20, 50, 255};

    spr.set_pixel(5, 5, fill);
    spr.add_shadow(1, 1, shadow);

    // Shadow projected at (6, 6)
    ASSERT_EQ(spr.get_pixel(6, 6), shadow);
    ASSERT_EQ(spr.get_pixel(5, 5), fill);
}

TEST(SemanticToolsTest, CleanPixelNoise) {
    sprite::Sprite spr(16, 16);
    core::Color solid{255, 255, 255, 255};

    // Draw main 3x3 block
    spr.draw_rectangle(4, 4, 3, 3, solid, true);
    // Draw isolated orphan noise pixel
    spr.set_pixel(12, 12, solid);

    ASSERT_FALSE(spr.get_pixel(12, 12).is_transparent());
    int cleaned = spr.clean_pixel_noise(1);
    ASSERT_EQ(cleaned, 1);
    // Orphan pixel removed
    ASSERT_TRUE(spr.get_pixel(12, 12).is_transparent());
    // Main block intact
    ASSERT_EQ(spr.get_pixel(4, 4), solid);
}

TEST(SemanticToolsTest, SymmetrizeRegion) {
    sprite::Sprite spr(16, 16);
    core::Color col{200, 100, 50, 255};

    // Draw asymmetrical shape in a 6x6 region at (4, 4)
    spr.set_pixel(4, 5, col);
    spr.set_pixel(5, 5, col);
    spr.create_region("face", core::Rect{4, 4, 6, 6});

    spr.symmetrize_region("face", 'x');

    // Right mirror of (4, 5) in a 6-wide box [4..9] is 9 - (4 - 4) = 9
    ASSERT_EQ(spr.get_pixel(9, 5), col);
    // Right mirror of (5, 5) is 9 - (5 - 4) = 8
    ASSERT_EQ(spr.get_pixel(8, 5), col);
}

TEST(SemanticToolsTest, IdentifyColorClusters) {
    sprite::Sprite spr(16, 16);
    core::Color c1{255, 0, 0, 255};
    core::Color c2{0, 0, 255, 255};

    // Cluster 1: 3x3 at (1, 1)
    spr.draw_rectangle(1, 1, 3, 3, c1, true);
    // Cluster 2: 3x3 at (8, 8)
    spr.draw_rectangle(8, 8, 3, 3, c2, true);

    auto clusters = spr.identify_color_clusters();
    ASSERT_EQ(clusters.size(), 2);
}
