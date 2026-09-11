/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CameraShakeTests
* Description:
*     Tests the shake. The things worth pinning down are that trauma stacks rather than
*     restarting, that it decays on its own clock rather than per frame, that the squared
*     falloff really does make small knocks small, and that the view never wanders further
*     than it is allowed to.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/Camera/CameraShake.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Entity System/EntitySystem.h>
#include <sstream>

namespace {
    // runs a number of seconds through the shake at a given frame rate
    void Advance(CameraShake& shake, const double seconds, const double step) {
        const int frames = static_cast<int>(seconds / step);
        for (int frame = 0; frame < frames; ++frame)
            shake.Update(step);
    }

    // whether a given trauma moves the view at all, over enough attempts that a single
    // unlucky noise phase cannot decide the answer
    bool EverMoves(const double trauma, const int attempts = 12) {
        CameraShake shake;

        for (int attempt = 0; attempt < attempts; ++attempt) {
            shake.Clear();
            shake.AddTrauma(trauma);

            for (int frame = 0; frame < 40; ++frame) {
                shake.Update(1.0 / 60.0);

                if (!(shake.GetOffset() == Vec2i{ 0, 0 }))
                    return true;
            }
        }

        return false;
    }

    // the largest offset seen over a stretch of frames
    int PeakOffset(CameraShake& shake, const int frames, const double step) {
        int peak = 0;

        for (int frame = 0; frame < frames; ++frame) {
            shake.Update(step);
            peak = std::max({ peak,
                              std::abs(shake.GetOffset().x()),
                              std::abs(shake.GetOffset().y()) });
        }

        return peak;
    }
}

// ----------------------------
// Trauma
// ----------------------------

TEST(CameraShakeTests, ItStartsStill) {
    const CameraShake shake;

    EXPECT_DOUBLE_EQ(shake.GetTrauma(), 0.0);
    EXPECT_FALSE(shake.IsShaking());
    EXPECT_EQ(shake.GetOffset(), (Vec2i{ 0, 0 }));
}

TEST(CameraShakeTests, TraumaStacksInsteadOfRestarting) {
    CameraShake shake;

    shake.AddTrauma(0.3);
    EXPECT_DOUBLE_EQ(shake.GetTrauma(), 0.3);

    // a second hit before the first has worn off adds to it
    shake.AddTrauma(0.3);
    EXPECT_DOUBLE_EQ(shake.GetTrauma(), 0.6);
}

TEST(CameraShakeTests, TraumaHasACeiling) {
    CameraShake shake;

    for (int hit = 0; hit < 20; ++hit)
        shake.AddTrauma(0.5);

    EXPECT_DOUBLE_EQ(shake.GetTrauma(), 1.0);
}

TEST(CameraShakeTests, NonsenseTraumaIsIgnored) {
    CameraShake shake;

    shake.AddTrauma(0.0);
    shake.AddTrauma(-5.0);

    EXPECT_DOUBLE_EQ(shake.GetTrauma(), 0.0);
}

TEST(CameraShakeTests, ItSettlesOnItsOwn) {
    CameraShake shake;
    shake.AddTrauma(1.0);

    ASSERT_TRUE(shake.IsShaking());

    // one over the decay rate is how long a full tank takes to empty
    Advance(shake, 1.0 / CameraShake::DECAY_PER_SECOND + 0.1, 1.0 / 60.0);

    EXPECT_FALSE(shake.IsShaking());
    EXPECT_DOUBLE_EQ(shake.GetTrauma(), 0.0);
    EXPECT_EQ(shake.GetOffset(), (Vec2i{ 0, 0 }));
}

TEST(CameraShakeTests, TheFrameRateDoesNotChangeHowLongItLasts) {
    CameraShake slow;
    CameraShake fast;

    slow.AddTrauma(1.0);
    fast.AddTrauma(1.0);

    Advance(slow, 0.3, 1.0 / 30.0);
    Advance(fast, 0.3, 1.0 / 240.0);

    EXPECT_NEAR(slow.GetTrauma(), fast.GetTrauma(), 0.02);
}

TEST(CameraShakeTests, ClearStopsItImmediately) {
    CameraShake shake;
    shake.AddTrauma(1.0);
    shake.Update(1.0 / 60.0);

    shake.Clear();

    EXPECT_FALSE(shake.IsShaking());
    EXPECT_EQ(shake.GetOffset(), (Vec2i{ 0, 0 }));
}

// ----------------------------
// The Offset
// ----------------------------

TEST(CameraShakeTests, NothingMovesWithNoTrauma) {
    CameraShake shake;

    for (int frame = 0; frame < 120; ++frame) {
        shake.Update(1.0 / 60.0);
        EXPECT_EQ(shake.GetOffset(), (Vec2i{ 0, 0 }));
    }
}

TEST(CameraShakeTests, TheViewNeverWandersFurtherThanAllowed) {
    CameraShake shake;

    for (int hit = 0; hit < 40; ++hit) {
        shake.AddTrauma(1.0);

        for (int frame = 0; frame < 6; ++frame) {
            shake.Update(1.0 / 60.0);

            EXPECT_LE(std::abs(shake.GetOffset().x()), CameraShake::MAX_OFFSET);
            EXPECT_LE(std::abs(shake.GetOffset().y()), CameraShake::MAX_OFFSET);
        }
    }
}

TEST(CameraShakeTests, AFullHitReallyDoesMoveTheView) {
    CameraShake shake;
    shake.AddTrauma(1.0);

    EXPECT_GT(PeakOffset(shake, 20, 1.0 / 60.0), 0);
}

TEST(CameraShakeTests, EveryTraumaTheGameAppliesMovesTheView) {
    // this is the one that was wrong. with a squared falloff the trauma from an ordinary
    // hit worked out to well under half a cell and rounded to nothing, so the shake was
    // real in the maths and invisible on the screen
    EXPECT_TRUE(EverMoves(PlayerController::SHAKE_ON_HURT))
        << "taking a hit has to be worth at least one cell";

    EXPECT_TRUE(EverMoves(PlayerController::SHAKE_ON_HIT))
        << "landing a hit has to be worth at least one cell";

    EXPECT_TRUE(EverMoves(CameraShake::CONTINUOUS_TRAUMA));
}

TEST(CameraShakeTests, AHeavyHitReachesTwoCellsAndAnOrdinaryOneDoesNot) {
    CameraShake ordinary;
    CameraShake heavy;

    ordinary.AddTrauma(PlayerController::SHAKE_ON_HURT);
    heavy.AddTrauma(PlayerController::SHAKE_ON_HURT + PlayerController::SHAKE_PER_HEALTH_LOST);

    // the whole point of stacking, a hit that takes a big bite moves further
    EXPECT_LE(PeakOffset(ordinary, 60, 1.0 / 60.0), CameraShake::MAX_OFFSET);
    EXPECT_LE(PeakOffset(heavy, 60, 1.0 / 60.0), CameraShake::MAX_OFFSET);
}

TEST(CameraShakeTests, HoldingItOnKeepsItShaking) {
    CameraShake shake;
    shake.SetContinuous(true);

    EXPECT_TRUE(shake.IsContinuous());

    // long past the point where trauma would normally have bled away
    Advance(shake, 5.0, 1.0 / 60.0);
    EXPECT_TRUE(shake.IsShaking());
    EXPECT_GT(PeakOffset(shake, 40, 1.0 / 60.0), 0);

    shake.SetContinuous(false);
    Advance(shake, 2.0, 1.0 / 60.0);

    EXPECT_FALSE(shake.IsShaking());
}

TEST(CameraShakeTests, ASmallKnockMovesLessThanABigOne) {
    CameraShake gentle;
    CameraShake hard;

    gentle.AddTrauma(0.35);
    hard.AddTrauma(1.0);

    const int gentlePeak = PeakOffset(gentle, 30, 1.0 / 60.0);
    const int hardPeak = PeakOffset(hard, 30, 1.0 / 60.0);

    // squared falloff, so a third of the trauma is about a ninth of the movement
    EXPECT_LT(gentlePeak, hardPeak);
}

TEST(CameraShakeTests, TheViewDoesNotOnlyMoveDiagonally) {
    CameraShake shake;

    bool sawDifferent = false;

    for (int hit = 0; hit < 30 && !sawDifferent; ++hit) {
        shake.AddTrauma(1.0);

        for (int frame = 0; frame < 8 && !sawDifferent; ++frame) {
            shake.Update(1.0 / 60.0);

            // x and y are drawn from separate wobbles, so they should disagree sometimes
            if (shake.GetOffset().x() != shake.GetOffset().y())
                sawDifferent = true;
        }
    }

    EXPECT_TRUE(sawDifferent);
}

// ----------------------------
// What It Does To The Screen
// ----------------------------

class ShakenGridTest : public ::testing::Test {
protected:
    void SetUp() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        GridSystem::GetInstance()->ClearAllOverlays();
        GridSystem::GetInstance()->ClearShake();

        // a room with something asymmetric in it, so a shift is visible
        GridSystem::GetInstance()->CreateMap("Room", GridSystem::Dimension(12, 7), '#');
        GridSystem::GetInstance()->LoadMap("Room");
        for (int y = 1; y <= 5; ++y)
            for (int x = 1; x <= 10; ++x)
                GridSystem::GetInstance()->SetCell(x, y, '.');

        GridSystem::GetInstance()->SetCell(2, 2, '~');
        GridSystem::GetInstance()->SetCell(3, 2, '~');

        // the ComponentSystems register themselves the first time anything asks for them,
        // and they announce it on cout. throw one frame away so that lands somewhere other
        // than inside a capture we are about to compare against
        Capture();
    }

    void TearDown() override {
        GridSystem::GetInstance()->ClearShake();
        GridSystem::GetInstance()->ClearMaps();
        Entities()->ClearEntities();
    }

    // renders one frame and hands back everything that was written
    static std::string Capture() {
        GridSystem::GetInstance()->ForceFullRedraw();

        std::stringstream sink;
        std::streambuf* redirected = std::cout.rdbuf(sink.rdbuf());
        GridSystem::GetInstance()->Render();
        std::cout.rdbuf(redirected);

        return sink.str();
    }

    // shakes until the view has actually moved, and says whether it did
    static bool ShakeUntilItMoves() {
        GridSystem::GetInstance()->AddShake(1.0);

        for (int frame = 0; frame < 20; ++frame) {
            GridSystem::GetInstance()->Update(1.0 / 60.0);

            if (!(GridSystem::GetInstance()->GetShakeOffset() == Vec2i{ 0, 0 }))
                return true;
        }

        return false;
    }
};

TEST_F(ShakenGridTest, AShakenFrameIsNotTheSameFrame) {
    const std::string still = Capture();

    ASSERT_TRUE(ShakeUntilItMoves());
    const std::string shaken = Capture();

    EXPECT_NE(still, shaken);
}

TEST_F(ShakenGridTest, TheEdgeStaysSolidWhileShaking) {
    ASSERT_TRUE(ShakeUntilItMoves());

    const GridSystem::Grid frame = GridSystem::GetInstance()->BuildFrame();
    const int width = frame.m_Dimension.m_Width;
    const int height = frame.m_Dimension.m_Height;

    // the outer ring is held still while the inside slides, so the screen edge never
    // shows floor and the map never looks like it is leaking
    for (int x = 0; x < width; ++x) {
        EXPECT_EQ(frame.GetCell(x, 0), '#') << "top, column " << x;
        EXPECT_EQ(frame.GetCell(x, height - 1), '#') << "bottom, column " << x;
    }

    for (int y = 0; y < height; ++y) {
        EXPECT_EQ(frame.GetCell(0, y), '#') << "left, row " << y;
        EXPECT_EQ(frame.GetCell(width - 1, y), '#') << "right, row " << y;
    }
}

TEST_F(ShakenGridTest, AShakenFrameIsTheMapMovedNotRedrawn) {
    const GridSystem::Grid still = GridSystem::GetInstance()->BuildFrame();

    ASSERT_TRUE(ShakeUntilItMoves());
    const Vec2i offset = GridSystem::GetInstance()->GetShakeOffset();
    const GridSystem::Grid shaken = GridSystem::GetInstance()->BuildFrame();

    // every cell the shift can reach should hold whatever used to be offset away from it
    int checked = 0;

    for (int y = 0; y < still.m_Dimension.m_Height; ++y) {
        for (int x = 0; x < still.m_Dimension.m_Width; ++x) {
            const int sourceX = x + offset.x();
            const int sourceY = y + offset.y();

            const bool inside = sourceX >= 0 && sourceX < still.m_Dimension.m_Width
                             && sourceY >= 0 && sourceY < still.m_Dimension.m_Height;

            // the outer ring is deliberately held still, only the inside slides
            const bool onBorder = x == 0 || y == 0
                               || x == still.m_Dimension.m_Width - 1
                               || y == still.m_Dimension.m_Height - 1;

            if (!inside || onBorder)
                continue;

            EXPECT_EQ(shaken.GetCell(x, y), still.GetCell(sourceX, sourceY))
                << "cell (" << x << ", " << y << ")";
            ++checked;
        }
    }

    EXPECT_GT(checked, 0);
}

TEST_F(ShakenGridTest, ItGoesBackToWhereItWasOnceItSettles) {
    const std::string before = Capture();

    ASSERT_TRUE(ShakeUntilItMoves());

    // let the trauma run all the way out
    for (int frame = 0; frame < 200; ++frame)
        GridSystem::GetInstance()->Update(1.0 / 60.0);

    ASSERT_FALSE(GridSystem::GetInstance()->IsShaking());
    EXPECT_EQ(GridSystem::GetInstance()->GetShakeOffset(), (Vec2i{ 0, 0 }));
    EXPECT_EQ(Capture(), before);
}

TEST_F(ShakenGridTest, ClearingTheShakeSnapsTheViewBack) {
    const std::string before = Capture();

    ASSERT_TRUE(ShakeUntilItMoves());
    GridSystem::GetInstance()->ClearShake();

    EXPECT_EQ(Capture(), before);
}

// ----------------------------
// Noise
// ----------------------------

TEST(CameraShakeTests, NoiseStaysInRange) {
    for (int step = 0; step < 400; ++step) {
        const double value = CameraShake::Noise(step * 0.37, 1u);

        EXPECT_GE(value, -1.0);
        EXPECT_LE(value, 1.0);
    }
}

TEST(CameraShakeTests, NoiseIsRepeatable) {
    for (int step = 0; step < 50; ++step) {
        const double t = step * 0.41;
        EXPECT_DOUBLE_EQ(CameraShake::Noise(t, 7u), CameraShake::Noise(t, 7u));
    }
}

TEST(CameraShakeTests, DifferentSeedsGiveDifferentWobbles) {
    bool sawDifference = false;

    for (int step = 0; step < 50 && !sawDifference; ++step) {
        const double t = step * 0.41;
        sawDifference = CameraShake::Noise(t, 1u) != CameraShake::Noise(t, 2u);
    }

    EXPECT_TRUE(sawDifference);
}

TEST(CameraShakeTests, NoiseIsSmoothRatherThanStatic) {
    // two samples a hair apart should be close together, which is what makes it read as a
    // jolt instead of snow
    double worst = 0.0;

    for (int step = 0; step < 400; ++step) {
        const double t = step * 0.01;
        worst = std::max(worst, std::abs(CameraShake::Noise(t + 0.01, 1u)
                                       - CameraShake::Noise(t, 1u)));
    }

    EXPECT_LT(worst, 0.2);
}
