#include <gtest/gtest.h>
#include <Systems/System Registry/SystemRegistry.h>
#include <Systems/system.h>

// Fixture to reset the GridSystem between tests
typedef GridSystem::Dimension Dim;

class GridSystemTest : public ::testing::Test {
protected:
    std::shared_ptr<GridSystem> gs;
    void SetUp() override {
        gs = GridSystem::GetInstance();
        gs->ClearMaps();
    }
    void TearDown() override {
        gs->ClearMaps();
    }
};

// -------------------------
// Grid Struct Tests
// -------------------------

TEST(GridTest, DefaultConstructor) {
    GridSystem::Grid grid;
    EXPECT_EQ(grid.m_Dimension.m_Width, 0);
    EXPECT_EQ(grid.m_Dimension.m_Height, 0);
    EXPECT_TRUE(grid.cells.empty());
}

TEST(GridTest, FillConstructor) {
    GridSystem::Grid grid(3, 2, '#');
    EXPECT_EQ(grid.m_Dimension.m_Width, 3);
    EXPECT_EQ(grid.m_Dimension.m_Height, 2);
    ASSERT_EQ(grid.cells.size(), 6);
    for (char c : grid.cells) {
        EXPECT_EQ(c, '#');
    }
}

TEST(GridTest, GetCellBounds) {
    GridSystem::Grid grid(2, 2, '*');
    // In-bounds
    EXPECT_EQ(grid.GetCell(0, 0), '*');
    EXPECT_EQ(grid.GetCell(1, 1), '*');
    // Out-of-bounds
    EXPECT_EQ(grid.GetCell(-1, 0), ' ');
    EXPECT_EQ(grid.GetCell(2, 1), ' ');
    EXPECT_EQ(grid.GetCell(0, -1), ' ');
    EXPECT_EQ(grid.GetCell(0, 2), ' ');
}

TEST(GridTest, SetCellInBoundsAndOut) {
    GridSystem::Grid grid(2, 2, '.');
    grid.SetCell(1, 1, 'X');
    EXPECT_EQ(grid.GetCell(1, 1), 'X');
    // Out-of-bounds should not crash or modify
    grid.SetCell(-1, 0, 'Y');
    EXPECT_EQ(grid.GetCell(0, 0), '.');
}

// -------------------------
// GridSystem Map Management Tests
// -------------------------

TEST_F(GridSystemTest, CreateAndLoadMap) {
    gs->CreateMap("test", Dim(4, 3), '.');
    EXPECT_TRUE(gs->LoadMap("test"));
    EXPECT_EQ(gs->GetWidth(), 4);
    EXPECT_EQ(gs->GetHeight(), 3);
}

TEST_F(GridSystemTest, LoadMapFailsWhenMissing) {
    EXPECT_FALSE(gs->LoadMap("missing"));
}

TEST_F(GridSystemTest, DeleteMapClearsActive) {
    gs->CreateMap("m", Dim(1, 1), '#');
    EXPECT_TRUE(gs->LoadMap("m"));
    gs->DeleteMap("m");
    EXPECT_EQ(gs->GetWidth(), 0);
    EXPECT_EQ(gs->GetHeight(), 0);
    EXPECT_FALSE(gs->LoadMap("m"));
}

TEST_F(GridSystemTest, ClearMapsRemovesAll) {
    gs->CreateMap("a", Dim(2, 2), '.');
    gs->CreateMap("b", Dim(3, 3), '*');
    gs->ClearMaps();
    EXPECT_FALSE(gs->LoadMap("a"));
    EXPECT_FALSE(gs->LoadMap("b"));
}

TEST_F(GridSystemTest, SetAndGetCellOnActiveMap) {
    gs->CreateMap("map1", Dim(2, 2), '.');
    gs->LoadMap("map1");
    EXPECT_EQ(gs->GetCell(0, 0), '.');
    gs->SetCell(1, 0, 'A');
    EXPECT_EQ(gs->GetCell(1, 0), 'A');
    EXPECT_EQ(gs->GetCell(5, 5), ' ');
}

// -------------------------
// Console Utilities
// -------------------------

TEST(GridSystemUtilTest, ClearConsoleSequence) {
    testing::internal::CaptureStdout();
    GridSystem::ClearConsole();
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_EQ(out, "\033[2J\033[H");
}

// -------------------------
// Render Logic Tests
// -------------------------

TEST_F(GridSystemTest, RenderOnlyWhenDirty) {
    gs->CreateMap("rtest", Dim(2, 2), '.');
    gs->LoadMap("rtest");

    // First render should print
    testing::internal::CaptureStdout();
    gs->Render();
    std::string out1 = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(out1.empty());

    // Second render without changes should print nothing
    testing::internal::CaptureStdout();
    gs->Render();
    std::string out2 = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(out2.empty());

    // Modify a cell to mark dirty, then render again
    gs->SetCell(0, 0, 'X');
    testing::internal::CaptureStdout();
    gs->Render();
    std::string out3 = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(out3.empty());
}

// -------------------------
// Main Entry
// -------------------------

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
