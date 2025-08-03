#include <gtest/gtest.h>
#include <Systems/system.h>


class TestSystem final : public System {
public:
    TestSystem() : System("TestSystem") {}
    ~TestSystem() override = default; 

    bool initCalled = false;
    void Init() override { initCalled = true; }
    void Update(double deltaTime) override {}
    void FixedUpdate() override {}
    void Render() override {}
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