#include "TestRunner.hpp"
#include "animation/Animation.hpp"
#include "animation/FrameDiffer.hpp"

using namespace pixelforge;

TEST(AnimationTest, FrameLifecycle) {
    animation::Animation anim(16, 16, "walk");
    ASSERT_EQ(anim.frame_count(), 0);
    ASSERT_TRUE(anim.empty());

    sprite::Sprite s1(16, 16);
    s1.set_pixel(2, 2, core::Color{255, 0, 0, 255});
    size_t idx0 = anim.add_frame(s1, 150, "idle");
    ASSERT_EQ(idx0, 0);
    ASSERT_EQ(anim.frame_count(), 1);
    ASSERT_EQ(anim.total_duration_ms(), 150);

    // Duplicate frame 0
    size_t idx1 = anim.duplicate_frame(0);
    ASSERT_EQ(idx1, 1);
    ASSERT_EQ(anim.frame_count(), 2);

    // Modify duplicate frame
    anim.frame(1).sprite.set_pixel(3, 3, core::Color{0, 255, 0, 255});
    ASSERT_TRUE(anim.frame(0).sprite.get_pixel(3, 3).is_transparent());
    ASSERT_FALSE(anim.frame(1).sprite.get_pixel(3, 3).is_transparent());

    // Reorder
    ASSERT_TRUE(anim.reorder_frame(0, 1));

    // Delete
    ASSERT_TRUE(anim.delete_frame(0));
    ASSERT_EQ(anim.frame_count(), 1);
}

TEST(AnimationTest, FrameDifferConsistency) {
    animation::Animation anim(16, 16);

    // Frame 0: body at (4, 4), left_leg at (4, 10)
    sprite::Sprite s0(16, 16);
    s0.draw_rectangle(4, 4, 4, 4, core::Color{100, 100, 200, 255}, true);
    s0.create_region("body", core::Rect{4, 4, 4, 4});

    s0.draw_rectangle(4, 10, 2, 4, core::Color{50, 50, 50, 255}, true);
    s0.create_region("left_leg", core::Rect{4, 10, 2, 4});

    anim.add_frame(s0, 100);

    // Frame 1: duplicate frame 0, but move left_leg by dx=+2
    anim.duplicate_frame(0);
    anim.frame(1).sprite.move_region("left_leg", 2, 0);

    // Compare
    auto diff = animation::FrameDiffer::compare(anim.frame(0), anim.frame(1), 0, 1);

    ASSERT_TRUE(diff.pixel_difference_count > 0);
    // Unchanged region: body
    bool body_unchanged = false;
    for (const auto& u : diff.unchanged_regions) {
        if (u == "body") body_unchanged = true;
    }
    ASSERT_TRUE(body_unchanged);

    // Changed region: left_leg
    bool leg_changed = false;
    for (const auto& c : diff.changed_regions) {
        if (c == "left_leg") leg_changed = true;
    }
    ASSERT_TRUE(leg_changed);
}

TEST(AnimationTest, RegionInterpolation) {
    animation::Animation anim(16, 16);

    // Frame 0: ball at (2, 2)
    sprite::Sprite s0(16, 16);
    s0.draw_rectangle(2, 2, 2, 2, core::Color{255, 255, 0, 255}, true);
    s0.create_region("ball", core::Rect{2, 2, 2, 2});
    anim.add_frame(s0);

    // Frame 1: blank placeholder
    anim.add_frame(sprite::Sprite(16, 16));

    // Frame 2: ball at (6, 2)
    sprite::Sprite s2(16, 16);
    s2.draw_rectangle(6, 2, 2, 2, core::Color{255, 255, 0, 255}, true);
    s2.create_region("ball", core::Rect{6, 2, 2, 2});
    anim.add_frame(s2);

    // Interpolate frame 1 between frame 0 and frame 2
    anim.interpolate_region("ball", 0, 2);

    // Midpoint ball should now be at (4, 2)
    ASSERT_TRUE(anim.frame(1).sprite.has_region("ball"));
    ASSERT_EQ(anim.frame(1).sprite.get_region("ball").bounds().x, 4);
    ASSERT_EQ(anim.frame(1).sprite.get_pixel(4, 2), core::Color(255, 255, 0, 255));
}
