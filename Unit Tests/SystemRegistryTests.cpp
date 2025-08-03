/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: SystemRegistryTests
* Description:
*      Tests the automatic registration of systems using the AUTO_REGISTER_SYSTEM macro.
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Systems/System Registry/SystemRegistry.h>
#include <Systems/system.h>

// Minimal test system
class TestAutoSystem : public System
{
public:
    TestAutoSystem() : System("TestAutoSystem") {}

    void Init() override {}
    void Update(double) override {}
    void FixedUpdate() override {}
    void Render() override {}
    void Shutdown() override {}

    // Return as shared_ptr for compatibility with macro
    static std::shared_ptr<TestAutoSystem> GetInstance()
    {
        static std::shared_ptr<TestAutoSystem> instance(new TestAutoSystem());
        return instance;
    }
};

// Macro to register system at static init time
REGISTER_SYSTEM(TestAutoSystem)

TEST(SystemRegistryTests, AutoRegisterRunsBeforeMain)
{
    const auto& systems = SystemRegistry::Instance().GetSystems();

    bool found = false;
    for (auto* sys : systems)
    {
        if (sys->GetName() == "TestAutoSystem")
        {
            found = true;
            break;
        }
    }

    EXPECT_TRUE(found) << "AUTO_REGISTER_SYSTEM did not register TestAutoSystem.";
}
