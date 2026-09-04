/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: HudTests
* Description:
*     Tests the health bar. It is drawn as runs of spaces on coloured backgrounds, so the
*     invariant that matters is that the blocks always add up to the same width no matter
*     what fraction goes in, including nonsense ones.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Systems/Hud/HudSystem.h>

namespace {
    // the bar is spaces on coloured backgrounds, so counting spaces measures its width
    int BarWidth(const std::string& bar) {
        return static_cast<int>(std::count(bar.begin(), bar.end(), ' '));
    }

    const std::string GREEN = "\x1b[42m";
    const std::string AMBER = "\x1b[43m";
    const std::string RED   = "\x1b[41m";
    const std::string WOUND = "\x1b[101m";
    const std::string EMPTY = "\x1b[100m";

    bool Contains(const std::string& bar, const std::string& code) {
        return bar.find(code) != std::string::npos;
    }
}

TEST(HudTests, ABarIsAlwaysTheSameWidth) {
    for (const double fraction : { 0.0, 0.1, 0.25, 0.5, 0.75, 0.99, 1.0 })
        EXPECT_EQ(BarWidth(HudSystem::BuildBar(fraction, fraction, 20)), 20)
            << "fraction " << fraction;
}

TEST(HudTests, NonsenseFractionsAreClampedRatherThanTrusted) {
    EXPECT_EQ(BarWidth(HudSystem::BuildBar(-5.0, -5.0, 12)), 12);
    EXPECT_EQ(BarWidth(HudSystem::BuildBar(9.0, 9.0, 12)), 12);

    // a trail behind the bar makes no sense, it gets pulled up to meet it
    EXPECT_EQ(BarWidth(HudSystem::BuildBar(0.8, 0.2, 12)), 12);
}

TEST(HudTests, AFullBarHasNothingEmptyInIt) {
    const std::string bar = HudSystem::BuildBar(1.0, 1.0, 16);

    EXPECT_TRUE(Contains(bar, GREEN));
    EXPECT_FALSE(Contains(bar, EMPTY));
    EXPECT_FALSE(Contains(bar, WOUND));
}

TEST(HudTests, AnEmptyBarHasNothingFilledInIt) {
    const std::string bar = HudSystem::BuildBar(0.0, 0.0, 16);

    EXPECT_TRUE(Contains(bar, EMPTY));
    EXPECT_FALSE(Contains(bar, GREEN));
    EXPECT_FALSE(Contains(bar, AMBER));
}

TEST(HudTests, TheBarChangesColourAsItDrains) {
    EXPECT_TRUE(Contains(HudSystem::BuildBar(1.00, 1.00, 16), GREEN));
    EXPECT_TRUE(Contains(HudSystem::BuildBar(0.40, 0.40, 16), AMBER));
    EXPECT_TRUE(Contains(HudSystem::BuildBar(0.10, 0.10, 16), RED));
}

TEST(HudTests, AWoundShowsWhatWasJustLost) {
    // the bar has dropped to a quarter but the trail has not caught up yet
    const std::string hit = HudSystem::BuildBar(0.25, 0.75, 20);

    EXPECT_EQ(BarWidth(hit), 20);
    EXPECT_TRUE(Contains(hit, WOUND));

    // once the trail catches up the wound is gone
    const std::string settled = HudSystem::BuildBar(0.25, 0.25, 20);
    EXPECT_FALSE(Contains(settled, WOUND));
}

TEST(HudTests, EveryBarEndsBackAtTheDefaultColour) {
    // otherwise whatever is drawn next inherits the bar's background
    for (const double fraction : { 0.0, 0.5, 1.0 }) {
        const std::string bar = HudSystem::BuildBar(fraction, 1.0, 10);
        EXPECT_EQ(bar.rfind("\x1b[0m"), bar.size() - 4) << "fraction " << fraction;
    }
}

TEST(HudTests, AZeroWidthBarIsEmptyRatherThanBroken) {
    const std::string bar = HudSystem::BuildBar(0.5, 0.5, 0);

    EXPECT_EQ(BarWidth(bar), 0);
}
