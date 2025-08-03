#include <gtest/gtest.h>
#include <Systems/system.h>

class TestSystem : public System {
public:
    TestSystem() : System("TestSystem") {}
    bool initCalled = false;
    void Init() override { initCalled = true; }
};

TEST(SystemTests, NameIsSetCorrectly) {
    TestSystem sys;
    EXPECT_EQ(sys.GetName(), "TestSystem");
}

TEST(SystemTests, InitIsCallable) {
    TestSystem sys;
    sys.Init();
    EXPECT_TRUE(sys.initCalled);
}
