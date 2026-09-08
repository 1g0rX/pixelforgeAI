#include "TestRunner.hpp"
#include "sprite/History.hpp"

using namespace pixelforge;

TEST(HistoryTest, UndoRedoFlow) {
    sprite::History hist;
    sprite::Sprite spr(8, 8);
    core::Color red{255, 0, 0, 255};
    core::Color blue{0, 0, 255, 255};

    ASSERT_FALSE(hist.can_undo());
    ASSERT_FALSE(hist.can_redo());

    // Action 1: set red at (0, 0)
    hist.record_state(spr, "draw_red");
    spr.set_pixel(0, 0, red);

    // Action 2: set blue at (1, 1)
    hist.record_state(spr, "draw_blue");
    spr.set_pixel(1, 1, blue);

    ASSERT_TRUE(hist.can_undo());
    ASSERT_EQ(spr.get_pixel(1, 1), blue);

    // Undo Action 2
    spr = hist.undo(spr);
    ASSERT_TRUE(spr.get_pixel(1, 1).is_transparent());
    ASSERT_EQ(spr.get_pixel(0, 0), red);
    ASSERT_TRUE(hist.can_redo());

    // Redo Action 2
    spr = hist.redo(spr);
    ASSERT_EQ(spr.get_pixel(1, 1), blue);

    // Undo Action 2 and Action 1
    spr = hist.undo(spr);
    spr = hist.undo(spr);
    ASSERT_TRUE(spr.get_pixel(0, 0).is_transparent());
    ASSERT_FALSE(hist.can_undo());
}

TEST(HistoryTest, Checkpoints) {
    sprite::History hist;
    sprite::Sprite spr(8, 8);
    spr.set_pixel(3, 3, core::Color{10, 20, 30, 255});

    hist.save_checkpoint("base_character", spr);
    ASSERT_TRUE(hist.has_checkpoint("base_character"));
    ASSERT_FALSE(hist.has_checkpoint("non_existent"));

    // Modify sprite
    spr.clear_pixel(3, 3);
    ASSERT_TRUE(spr.get_pixel(3, 3).is_transparent());

    // Restore checkpoint
    auto restored = hist.restore_checkpoint("base_character");
    ASSERT_TRUE(restored.has_value());
    ASSERT_EQ(restored->get_pixel(3, 3), core::Color(10, 20, 30, 255));
}
