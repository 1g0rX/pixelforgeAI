#include "TestRunner.hpp"
#include "sprite/Palette.hpp"

using namespace pixelforge;

TEST(PaletteTest, HexConversion) {
    auto c1 = core::Color::from_hex("#FF0000");
    ASSERT_EQ(c1.r, 255);
    ASSERT_EQ(c1.g, 0);
    ASSERT_EQ(c1.b, 0);
    ASSERT_EQ(c1.a, 255);
    ASSERT_EQ(c1.to_hex(), "#ff0000");

    auto c2 = core::Color::from_hex("#0F0");
    ASSERT_EQ(c2.r, 0);
    ASSERT_EQ(c2.g, 255);
    ASSERT_EQ(c2.b, 0);

    auto c3 = core::Color::from_hex("#0000FF80");
    ASSERT_EQ(c3.b, 255);
    ASSERT_EQ(c3.a, 128);
}

TEST(PaletteTest, ColorManagement) {
    sprite::Palette pal;
    pal.clear();
    ASSERT_EQ(pal.size(), 0);

    core::Color c{12, 34, 56, 255};
    size_t idx1 = pal.add_color(c);
    ASSERT_EQ(idx1, 0);
    ASSERT_EQ(pal.size(), 1);

    // Adding same color returns existing index
    size_t idx2 = pal.add_color(c);
    ASSERT_EQ(idx2, 0);
    ASSERT_EQ(pal.size(), 1);

    ASSERT_TRUE(pal.has_color(c));
    ASSERT_EQ(pal.get_color(0), c);

    ASSERT_TRUE(pal.remove_color(c));
    ASSERT_EQ(pal.size(), 0);
}

TEST(PaletteTest, NearestColorSearch) {
    sprite::Palette pal({
        core::Color{0, 0, 0, 255},       // 0: Black
        core::Color{255, 0, 0, 255},     // 1: Red
        core::Color{0, 255, 0, 255},     // 2: Green
        core::Color{0, 0, 255, 255}      // 3: Blue
    });

    core::Color dark_red{200, 20, 20, 255};
    size_t nearest = pal.find_nearest_index(dark_red);
    ASSERT_EQ(nearest, 1); // Should match Red

    core::Color dark_blue{10, 10, 220, 255};
    ASSERT_EQ(pal.find_nearest_index(dark_blue), 3);
}

TEST(PaletteTest, Quantization) {
    sprite::Palette pal({
        core::Color{0, 0, 0, 255},
        core::Color{10, 10, 10, 255},
        core::Color{20, 20, 20, 255},
        core::Color{200, 0, 0, 255},
        core::Color{210, 0, 0, 255},
        core::Color{0, 200, 0, 255}
    });
    ASSERT_EQ(pal.size(), 6);

    pal.quantize(3);
    ASSERT_EQ(pal.size(), 3);
}

TEST(PaletteTest, Presets) {
    auto p8 = sprite::Palette::pico8();
    ASSERT_EQ(p8.size(), 17); // 1 transparent + 16 pico8 colors

    auto gb = sprite::Palette::gameboy();
    ASSERT_EQ(gb.size(), 5);  // 1 transparent + 4 shades

    auto db16 = sprite::Palette::dawnbringer16();
    ASSERT_EQ(db16.size(), 17);
}
